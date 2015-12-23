/*
 * File:   ReceiveThread.h
 * Author: daniel
 *
 * Created on May 16, 2015, 12:49 PM
 */

#ifndef RTPLISTENER_H
#define	RTPLISTENER_H

#include <thread>

#include "PlaybackListener.h"
#include "ParticipantDatabase.h"
#include "RTPBufferHandler.h"
#include "NetworkWrapper.h"

/*!
 * Listening-thread for incoming RTP-packages
 *
 * This class starts a new thread which writes all received RTP-packages to the RTPBuffer
 */
class RTPListener : public PlaybackListener
{
public:
    /*!
     * Constructs a new RTPListener
     *
     * \param wrapper The NetworkWrapper to use for receiving packages
     *
     * \param buffer The RTPBuffer to write into
     *
     * \param receiveBufferSize The maximum size (in bytes) a RTP-package can fill, according to the configuration
     *
     */
    RTPListener(std::shared_ptr<NetworkWrapper> wrapper, std::shared_ptr<RTPBufferHandler> buffer, unsigned int receiveBufferSize);
    RTPListener(const RTPListener& orig);
    ~RTPListener();

    /*!
     * Shuts down the receive-thread
     */
    void shutdown();

    /*!
     * Starts the receive-thread
     */
    void startUp();
    
    /*!
     * Starts the receive-thread
     */
    void onPlaybackStart();
    
    /*!
     * Shuts down the receive-thread
     */
    void onPlaybackStop();

private:
    std::shared_ptr<NetworkWrapper> wrapper;
    std::shared_ptr<RTPBufferHandler> buffer;
    RTPPackageHandler rtpHandler;
    std::thread receiveThread;
    bool threadRunning = false;
    bool firstPackage = false;
    //for jitter-calculation
    uint32_t lastDelay;

    /*!
     * Method called in the parallel thread, receiving packages and writing them into RTPBuffer
     */
    void runThread();
    
    /*!
     * NOTE: is only called from #runThread()
     * 
     * \param sentTimestamp the RTP-timestamp of the remote device read from the RTPHeader
     * 
     * \param receptionTimestamp the RTP-timestamp of this device of the moment of reception
     * 
     * \return the interarrival-jitter for RTP-packages
     */
    float calculateInterarrivalJitter(uint32_t sentTimestamp, uint32_t receptionTimestamp);
    
    /*!
     * Calculates the new extended highest sequence number for the received package
     */
    uint32_t calculateExtendedHighestSequenceNumber(const uint16_t receivedSequenceNumber) const;
};

#endif	/* RTPLISTENER_H */

