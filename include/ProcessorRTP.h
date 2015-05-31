#ifndef PROCESSORRTP_H

#define	PROCESSORRTP_H
#include <string>
#include "AudioProcessor.h"
#include "RTPPackage.h"
#include "NetworkWrapper.h"

class ProcessorRTP : AudioProcessor
{
public:
	ProcessorRTP(std::string name, NetworkWrapper *networkwrapper);
	void processInputData(void *inputBuffer, const unsigned int inputBufferByteSize, StreamData *userData);
	void processOutputData(void *outputBuffer, const unsigned int outputBufferByteSize, StreamData *userData);
private:
	NetworkWrapper *networkObject;
	RTPPackage *rtpPackage = NULL;
};
#endif