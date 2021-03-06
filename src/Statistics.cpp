/*
 * File:   Statistics.cpp
 * Author: daniel
 *
 * Created on June 30, 2015, 5:06 PM
 */
#include <fstream>

#include "Statistics.h"
#include "Logger.h"

using namespace ohmcomm;

long Statistics::counters[] = {0};
std::vector<ProfilingAudioProcessor*> Statistics::audioProcessorStatistics;

void Statistics::incrementCounter(int counterIndex, long byValue)
{
    counters[counterIndex] += byValue;
}

void Statistics::setCounter(int counterIndex, long newValue)
{
    counters[counterIndex] = newValue;
}

void Statistics::maxCounter(int counterIndex, long newValue)
{
    if(newValue > counters[counterIndex])
    {
        counters[counterIndex] = newValue;
    }
}

void Statistics::printStatisticsToFile(const std::string fileName)
{
    std::ofstream fileStream(fileName.c_str(), std::ios_base::out|std::ios_base::trunc);
    if(fileStream.is_open())
    {
        printStatistics(fileStream);
    }
    else
    {
        ohmcomm::error("Statistics") << "Error while opening log-file!" << ohmcomm::endl;
    }
    fileStream.close();
    if(fileStream.fail())
    {
        ohmcomm::error("Statistics") << "Error while writing statistics!" << ohmcomm::endl;
    }
}

void Statistics::addProfiler(ProfilingAudioProcessor* profiler)
{
    Statistics::audioProcessorStatistics.push_back(profiler);
}


void Statistics::removeProfiler(ProfilingAudioProcessor* profiler)
{
    for (unsigned int i = 0; i < Statistics::audioProcessorStatistics.size(); i++)
    {
        if (Statistics::audioProcessorStatistics.at(i) ==profiler)
        {
            Statistics::audioProcessorStatistics.erase(Statistics::audioProcessorStatistics.begin() + i);
        }
    }
}


void Statistics::removeAllProfilers()
{
    Statistics::audioProcessorStatistics.clear();
}

void Statistics::resetStatistics()
{
    for(unsigned char i = 0; i < 20; ++i)
    {
        Statistics::counters[i] = 0;
    }
    removeAllProfilers();
}

void Statistics::printStatistics(std::ostream& outputStream)
{
    const double seconds = counters[TOTAL_ELAPSED_MILLISECONDS] / 1000.0;
    if(seconds == 0)
    {
        outputStream << "Couldn't print statistics" << std::endl;
        return;
    }
    outputStream << std::endl;
    outputStream << "Ran " << counters[TOTAL_ELAPSED_MILLISECONDS] << " ms (" << seconds << " s)" << std::endl;
    //Audio statistics
    outputStream << std::endl;
    outputStream << "+++ Audio statistics +++" << std::endl;
    outputStream << "Recorded " << counters[COUNTER_PAYLOAD_BYTES_RECORDED] << " bytes ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECORDED]) << ") of audio-data ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECORDED]/seconds) << "/s)" << std::endl;
    outputStream << "Recorded " << counters[COUNTER_FRAMES_RECORDED] << " audio-frames ("
            << (counters[COUNTER_FRAMES_RECORDED]/seconds) << " fps)" << std::endl;
    outputStream << "Played " << counters[COUNTER_PAYLOAD_BYTES_OUTPUT] << " bytes ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_OUTPUT]) << ") of audio-data ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_OUTPUT]/seconds) << "/s)" << std::endl;
    outputStream << "Played " << counters[COUNTER_FRAMES_OUTPUT] << " audio-frames ("
            << (counters[COUNTER_FRAMES_OUTPUT]/seconds) << " fps)" << std::endl;
    //Network statistics
    outputStream << std::endl;
    outputStream << "+++ Network statistics +++" << std::endl;
    outputStream << "Sent " << counters[COUNTER_PACKAGES_SENT] << " packages ("
            << (counters[COUNTER_PACKAGES_SENT]/seconds) << " per second)" << std::endl;
    outputStream << "Sent " << (counters[COUNTER_PAYLOAD_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT]) << " bytes ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT]) << ") in total ("
            << Utility::prettifyByteSize((counters[COUNTER_PAYLOAD_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT])/seconds) << "/s)"
            << std::endl;
    outputStream << "Sent " << counters[COUNTER_PAYLOAD_BYTES_SENT] << " bytes ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT]) << ") of audio-data ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT]/seconds) << "/s)" << std::endl;
    outputStream << "Sent " << counters[COUNTER_HEADER_BYTES_SENT] << " bytes ("
            << Utility::prettifyByteSize(counters[COUNTER_HEADER_BYTES_SENT]) << ") of RTP-header ("
            << Utility::prettifyByteSize(counters[COUNTER_HEADER_BYTES_SENT]/seconds) << "/s)" << std::endl;
    outputStream << "Sent " << counters[COUNTER_HEADER_BYTES_SENT] << " of "
            << counters[COUNTER_PAYLOAD_BYTES_SENT] << " bytes as overhead ("
            << Utility::prettifyPercentage(counters[COUNTER_HEADER_BYTES_SENT] / (double) counters[COUNTER_PAYLOAD_BYTES_SENT])
            << "%)" << std::endl;
    outputStream << "Received " << counters[COUNTER_PACKAGES_RECEIVED] << " packages ("
            << (counters[COUNTER_PACKAGES_RECEIVED]/seconds) << " per second)" << std::endl;
    outputStream << "Received " << (counters[COUNTER_PAYLOAD_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED]) << " bytes ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED]) << ") in total ("
            << Utility::prettifyByteSize((counters[COUNTER_PAYLOAD_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED])/seconds) << "/s)"
            << std::endl;
    outputStream << "Received " << counters[COUNTER_PAYLOAD_BYTES_RECEIVED] << " bytes ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED]) << ") of audio-data ("
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED]/seconds) << "/s)" << std::endl;
    outputStream << "Received " << counters[COUNTER_HEADER_BYTES_RECEIVED] << " bytes ("
            << Utility::prettifyByteSize(counters[COUNTER_HEADER_BYTES_RECEIVED]) << ") of RTP-header ("
            << Utility::prettifyByteSize(counters[COUNTER_HEADER_BYTES_RECEIVED]/seconds) << "/s)" << std::endl;
    outputStream << "Received " << counters[COUNTER_HEADER_BYTES_RECEIVED] << " of "
            << counters[COUNTER_PAYLOAD_BYTES_RECEIVED] << " bytes as overhead ("
            << Utility::prettifyPercentage(counters[COUNTER_HEADER_BYTES_RECEIVED] / (double) counters[COUNTER_PAYLOAD_BYTES_RECEIVED])
            << "%)" << std::endl;
    outputStream << "Lost " << counters[COUNTER_PACKAGES_LOST] << " RTP-packages ("
            << (counters[COUNTER_PACKAGES_LOST]/seconds) << " packages per second)" << std::endl;
    //Buffer statistics
    outputStream << std::endl;
    outputStream << "+++ Buffer statistics +++" << std::endl;
    outputStream << "Maximum buffer usage was " << counters[RTP_BUFFER_MAXIMUM_USAGE] << " of "
            << counters[RTP_BUFFER_LIMIT] << " packages ("
            << Utility::prettifyPercentage(counters[RTP_BUFFER_MAXIMUM_USAGE]/(double)counters[RTP_BUFFER_LIMIT]) << "%)"
            << std::endl;
    //Compression statistics
    //TODO is wrong, does not account for silence-packages
    outputStream << std::endl;
    outputStream << "+++ Compression statistics +++" << std::endl;
    outputStream << "Compressed " << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECORDED]) << " of audio-data into "
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_SENT]) << " ("
            << Utility::prettifyPercentage(1 - (counters[COUNTER_PAYLOAD_BYTES_SENT] / (double) counters[COUNTER_PAYLOAD_BYTES_RECORDED]))
            << "% compression)" << std::endl;
    outputStream << "Decompressed " << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_OUTPUT]) << " of audio-data out of "
            << Utility::prettifyByteSize(counters[COUNTER_PAYLOAD_BYTES_RECEIVED]) << " ("
            << Utility::prettifyPercentage(1.0 - (counters[COUNTER_PAYLOAD_BYTES_RECEIVED] / (double) counters[COUNTER_PAYLOAD_BYTES_OUTPUT]))
            << "% decompression)" << std::endl;

    // AudioProcessor statistics
    Statistics::printAudioProcessorStatistic(outputStream);
    
    //RTCP statistics
    Statistics::printRTCPStatistics(outputStream);
    
    outputStream << std::endl;
}

void Statistics::printAudioProcessorStatistic(std::ostream& outputStream)
{
    if(Statistics::audioProcessorStatistics.empty())
    {
        return;
    }
    // AudioProcessor statistics
    outputStream << std::endl;
    outputStream << "+++ AudioProcessor statistics +++" << std::endl;
    for(ProfilingAudioProcessor* profiler : Statistics::audioProcessorStatistics)
    {
        double inputAverage = profiler->getTotalInputTime() / (double)profiler->getTotalCount();
        double outputAverage = profiler->getTotalOutputTime() / (double)profiler->getTotalCount();
        outputStream << std::endl << profiler->getName() << std::endl;
        outputStream << "\tProcessing audio-input took " << profiler->getTotalInputTime()
                << " microseconds in total (" << inputAverage << " microseconds per call)" << std::endl;
        outputStream << "\tProcessing audio-output took  " << profiler->getTotalOutputTime()
                << " microseconds in total (" << outputAverage << " microseconds per call)" << std::endl;
    }
}

void Statistics::printRTCPStatistics(std::ostream& outputStream)
{
    const double seconds = counters[TOTAL_ELAPSED_MILLISECONDS] / 1000.0;
    outputStream << std::endl;
    outputStream << "+++ RTCP statistics +++" << std::endl;
    
    outputStream << "Sent " << counters[RTCP_PACKAGES_SENT] << " RTCP packages ("
            << (counters[RTCP_PACKAGES_SENT]/seconds) << " per second)" << std::endl;
    outputStream << "Sent " << counters[RTCP_BYTES_SENT] << " bytes of RTCP headers ("
            << Utility::prettifyByteSize(counters[RTCP_BYTES_SENT]) << ") in total ("
            << Utility::prettifyByteSize(counters[RTCP_BYTES_SENT]/seconds) << "/s)"
            << std::endl;
    outputStream << "Used " << Utility::prettifyPercentage(counters[RTCP_BYTES_SENT] / 
            (double)(counters[RTCP_BYTES_SENT] + counters[COUNTER_HEADER_BYTES_SENT] + counters[COUNTER_PAYLOAD_BYTES_SENT]))
            << "% of outgoing bandwidth for RTCP" << std::endl;
    
    outputStream << "Received " << counters[RTCP_PACKAGES_RECEIVED] << " RTCP packages ("
            << (counters[RTCP_PACKAGES_RECEIVED]/seconds) << " per second)" << std::endl;
    outputStream << "Received " << counters[RTCP_BYTES_RECEIVED] << " bytes of RTCP headers ("
            << Utility::prettifyByteSize(counters[RTCP_BYTES_RECEIVED]) << ") in total ("
            << Utility::prettifyByteSize(counters[RTCP_BYTES_RECEIVED]/seconds) << "/s)"
            << std::endl;
    outputStream << "Used " << Utility::prettifyPercentage(counters[RTCP_BYTES_RECEIVED] / 
            (double)(counters[RTCP_BYTES_RECEIVED] + counters[COUNTER_HEADER_BYTES_RECEIVED] + counters[COUNTER_PAYLOAD_BYTES_RECEIVED]))
            << "% of incoming bandwidth for RTCP" << std::endl;
}

