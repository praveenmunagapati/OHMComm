/*
 * File:   RTPBuffer.cpp
 * Author: daniel
 *
 * Created on March 28, 2015, 12:27 PM
 */


#include "rtp/RTPBuffer.h"
#include "rtp/ParticipantDatabase.h"

using namespace ohmcomm::rtp;

RTPBuffer::RTPBuffer(uint32_t ssrc, uint16_t maxCapacity, uint16_t maxDelay, uint16_t minBufferPackages) : PlayoutPointAdaption(200, minBufferPackages),
    ssrc(ssrc), capacity(maxCapacity), maxDelay(std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::milliseconds(maxDelay)))
{
    nextReadIndex = 0;
    ringBuffer = new RTPBufferPackage[maxCapacity];
    size = 0;
    minSequenceNumber = 0;
    Statistics::setCounter(Statistics::RTP_BUFFER_LIMIT, maxCapacity);
}

RTPBuffer::~RTPBuffer()
{
    delete [] ringBuffer;
}

RTPBufferStatus RTPBuffer::addPackage(const RTPPackageHandler &package, unsigned int contentSize)
{
    std::lock_guard<std::mutex> guard(bufferMutex);
    const RTPHeader *receivedHeader = package.getRTPPackageHeader();
    if(minSequenceNumber == 0 || receivedHeader->isMarked())
    {
        //if we receive our first package, we need to set minSequenceNumber
        //same for a marked package after a silent period (for DTX)
        //TODO same for any first package after a silent period (to correctly handle lost marked package and large consecutive losses)
        minSequenceNumber = receivedHeader->getSequenceNumber();
    }

    //we need to check for upper limit of range, because at some point a wrap around UINT16_MAX is expected behavior
    // -> if minSequenceNumber is larger than (UINT16_MAX - capacity), sequence_number around zero have to be allowed for
    if(minSequenceNumber < (UINT16_MAX - capacity) && receivedHeader->getSequenceNumber() < minSequenceNumber)
    {
        //late loss
        //discard package, because it is older than the minimum sequence number to hold
        packageReceived(true);
        return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
    }

    if(size == capacity)
    {
        //buffer is full
        return RTPBufferStatus::RTP_BUFFER_INPUT_OVERFLOW;
    }
    if(receivedHeader->getSequenceNumber() - minSequenceNumber >= capacity)
    {
        //should never occur: package is far too new -> we have now choice but to discard it without getting into an undetermined state
        //TODO can occur if playout gets somehow stuck -> overwrite old packages (see alternative buffer)
        return RTPBufferStatus::RTP_BUFFER_INPUT_OVERFLOW;
    }
    uint16_t newWriteIndex = calculateIndex(nextReadIndex, receivedHeader->getSequenceNumber()-minSequenceNumber);
    //write package-data into buffer
    ringBuffer[newWriteIndex].isValid = true;
    ringBuffer[newWriteIndex].header = *receivedHeader;
    if(ringBuffer[newWriteIndex].packageContent == nullptr)
    {
        //allocate new buffer with the current content-size
        ringBuffer[newWriteIndex].bufferSize = contentSize;
        ringBuffer[newWriteIndex].packageContent = malloc(contentSize);
    }
    else if(ringBuffer[newWriteIndex].bufferSize < contentSize)
    {
        //reallocate buffer, because the content would not fit
        ringBuffer[newWriteIndex].bufferSize = contentSize;
        ringBuffer[newWriteIndex].packageContent = realloc(ringBuffer[newWriteIndex].packageContent, contentSize);
    }
    //save timestamp of reception
    ringBuffer[newWriteIndex].receptionTimestamp = std::chrono::steady_clock::now();
    ringBuffer[newWriteIndex].contentSize = contentSize;
    memcpy(ringBuffer[newWriteIndex].packageContent, package.getRTPPackageData(), contentSize);
    //update size
    size++;
    Statistics::maxCounter(Statistics::RTP_BUFFER_MAXIMUM_USAGE, size);
    packageReceived(false);
    return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
}

RTPBufferStatus RTPBuffer::readPackage(RTPPackageHandler &package)
{
    std::lock_guard<std::mutex> guard(bufferMutex);
    if(!isAdaptionBufferFilled())
    {
        //buffer has insufficient fill level
        //return concealment package
        createConcealmentPackage(package);
        //we do not increase the minimum sequence number here, because we want to stretch the play-out delay
        //for that, we need to insert, not replace packages
        return RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW;
    }
    //need to search for oldest valid package, newer than minSequenceNumber and newer than currentTimestamp - maxDelay
    uint16_t index = nextReadIndex;
    const std::chrono::steady_clock::time_point currentTimestamp = std::chrono::steady_clock::now();
    while(incrementIndex(index) != nextReadIndex)
    {
        //check whether package is too delayed
        if(ringBuffer[index].isValid == true && ringBuffer[index].receptionTimestamp + maxDelay < currentTimestamp)
        {
            //package is valid but too old, invalidate and skip
            ringBuffer[index].isValid = false;
        }
        else if(ringBuffer[index].isValid == true && ringBuffer[index].header.getSequenceNumber() >= minSequenceNumber)
        {
            nextReadIndex = index;
            break;
        }
        index = incrementIndex(index);
    }
    //This copies the content of ringBuffer[readIndex] into package
    RTPBufferPackage *bufferPack = &(ringBuffer[nextReadIndex]);
    if(bufferPack->isValid == false)
    {
        //no valid packages found -> buffer is empty
        //return concealment package
        concealLoss(package, minSequenceNumber);
        //only accept newer packages (at least one sequence number more than the dummy package)
        //but skip check for first package
        if(minSequenceNumber != 0)
            minSequenceNumber = (minSequenceNumber + 1) % UINT16_MAX;
        return RTPBufferStatus::RTP_BUFFER_OUTPUT_UNDERFLOW;
    }

    char *packageBuffer = (char *)package.getWriteBuffer(bufferPack->contentSize + sizeof(bufferPack->header));
    memcpy(packageBuffer, &(bufferPack->header), sizeof(bufferPack->header));
    memcpy(packageBuffer + sizeof(bufferPack->header), bufferPack->packageContent, bufferPack->contentSize);
    package.setActualPayloadSize(bufferPack->contentSize);

    //Invalidate buffer-entry
    bufferPack->isValid = false;
    //Increment Index, decrease size
    nextReadIndex = incrementIndex(nextReadIndex);
    size--;
    //we lost all packages between the last read and this one, so we subtract the sequence numbers
    if(ParticipantDatabase::isInDatabase(ssrc))
    {
        //don't create new remote here, if it doesn't exist anymore
        ParticipantDatabase::remote(ssrc).packagesLost += (bufferPack->header.getSequenceNumber() - minSequenceNumber)%UINT16_MAX;
    }
    Statistics::incrementCounter(Statistics::COUNTER_PACKAGES_LOST, (bufferPack->header.getSequenceNumber() - minSequenceNumber)%UINT16_MAX);
    //only accept newer packages (at least one sequence number more than last read package)
    minSequenceNumber = (bufferPack->header.getSequenceNumber() + 1) % UINT16_MAX;
    return RTPBufferStatus::RTP_BUFFER_ALL_OKAY;
}

unsigned int RTPBuffer::getSize() const
{
    return size;
}

bool RTPBuffer::repeatLastPackage(RTPPackageHandler& package, const uint16_t packageSequenceNumber)
{
    //reverse iterate the buffer to get to the position for the given sequence-number
    uint16_t index = nextReadIndex;
    while(index != incrementIndex(nextReadIndex))
    {
        if(ringBuffer[index].header.getSequenceNumber() == packageSequenceNumber)
        {
            RTPBufferPackage *bufferPack = &(ringBuffer[index]);
            char *packageBuffer = (char *)package.getWriteBuffer(bufferPack->contentSize + sizeof(bufferPack->header));
            memcpy(packageBuffer, &(bufferPack->header), sizeof(bufferPack->header));
            memcpy(packageBuffer + sizeof(bufferPack->header), bufferPack->packageContent, bufferPack->contentSize);
            package.setActualPayloadSize(bufferPack->contentSize);
            return true;
        }
        index = index == 0 ? capacity : index-1;
    }
    return false;
}

uint16_t RTPBuffer::calculateIndex(uint16_t index, uint16_t offset)
{
    return (index + offset) % capacity;
}

uint16_t RTPBuffer::incrementIndex(uint16_t index)
{
    return (index+1) % capacity;
}