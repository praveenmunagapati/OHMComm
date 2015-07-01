/* 
 * File:   Statistics.h
 * Author: daniel
 *
 * Created on June 30, 2015, 5:06 PM
 */

#ifndef STATISTICS_H
#define	STATISTICS_H

#include <iostream>
#include <string>

/*!
 * Class to collect and print statistical information
 */
class Statistics
{
public:
    static const int COUNTER_PACKAGES_SENT;
    static const int COUNTER_PACKAGES_RECEIVED;
    static const int COUNTER_PACKAGES_LOST;
    static const int COUNTER_FRAMES_SENT;
    static const int COUNTER_FRAMES_RECORDED;
    static const int COUNTER_FRAMES_RECEIVED;
    static const int COUNTER_FRAMES_OUTPUT;
    static const int COUNTER_HEADER_BYTES_SENT;
    static const int COUNTER_HEADER_BYTES_RECEIVED;
    static const int COUNTER_PAYLOAD_BYTES_SENT;
    static const int COUNTER_PAYLOAD_BYTES_RECORDED;
    static const int COUNTER_PAYLOAD_BYTES_RECEIVED;
    static const int COUNTER_PAYLOAD_BYTES_OUTPUT;
    static const int TOTAL_ELAPSED_MILLISECONDS;
    static const int RTP_BUFFER_MAXIMUM_USAGE;
    static const int RTP_BUFFER_LIMIT;
    
    /*!
     * Increments the given counter by the value provided
     * 
     * \param counterIndex The key of the counter to increment
     * 
     * \param byValue The value to increment by, defaults to 1
     */
    static void incrementCounter(int counterIndex, long byValue = 1);
    
    /*!
     * Sets the given counter to the value provided
     * 
     * \param counterIndex The key to the counter
     * 
     * \param newValue The value to set
     */
    static void setCounter(int counterIndex, long newValue);
    
    /*!
     * Sets the given counter to the maximum of its old value and newValue
     * 
     * \param counterIndex The key to the counter
     * 
     * \param newValue The new value to compare and possibly set
     */
    static void maxCounter(int counterIndex, long newValue);
    
    /*!
     * Prints some general statistical information
     */
    static void printStatistics();
private:

    static long counters[20];
    
    static double prettifyPercentage(double percentage);
    
    static std::string prettifyByteSize(double byteSize);
};

#endif	/* STATISTICS_H */

