/* 
 * File:   AMRCodec.h
 * Author: daniel
 *
 * Created on March 2, 2016, 1:39 PM
 */
#ifdef AMR_ENCODER_HEADER //Only compile if AMR is linked
#ifndef AMRCODEC_H
#define	AMRCODEC_H

#include AMR_ENCODER_HEADER
#include AMR_DECODER_HEADER

#include "processors/AudioProcessor.h"

namespace ohmcomm
{
    namespace codecs
    {

        /*!
         * AMR-NB (narrow band) codec using the opencore-AMR implementation
         * 
         * RTP payload-types according to https://tools.ietf.org/html/rfc4867
         */
        class AMRCodec : public AudioProcessor
        {
        public:
            AMRCodec(const std::string& name);
            virtual ~AMRCodec();

            virtual unsigned int getSupportedAudioFormats() const;
            virtual unsigned int getSupportedSampleRates() const;
            virtual const std::vector<int> getSupportedBufferSizes(unsigned int sampleRate) const;
            virtual PayloadType getSupportedPlayloadType() const;

            virtual void configure(const AudioConfiguration& audioConfig, const std::shared_ptr<ConfigurationMode> configMode, const uint16_t bufferSize);

            virtual unsigned int processInputData(void* inputBuffer, const unsigned int inputBufferByteSize, StreamData* userData);
            virtual unsigned int processOutputData(void* outputBuffer, const unsigned int outputBufferByteSize, StreamData* userData);

            virtual bool cleanUp();
        private:
            void* amrEncoder;
            void* amrDecoder;

        };
    }
}
#endif	/* AMRCODEC_H */
#endif