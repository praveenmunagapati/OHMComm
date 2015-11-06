/*
 * File:   TestRTCP.cpp
 * Author: daniel
 *
 * Created on August 16, 2015, 10:49 AM
 */

#include "TestRTCP.h"
#include "RTPPackageHandler.h"

TestRTCP::TestRTCP()
{
    TEST_ADD(TestRTCP::testSenderReportPackage);
    TEST_ADD(TestRTCP::testReceiverReportPackage);
    TEST_ADD(TestRTCP::testSourceDescriptionPacakge);
    TEST_ADD(TestRTCP::testByePackage);
    TEST_ADD(TestRTCP::testAppDefinedPackage);
    TEST_ADD(TestRTCP::testIsRTCPPackage);
}

void TestRTCP::testSenderReportPackage()
{
    uint32_t packageCount = 42;

    //create package
    RTCPHeader header(testSSRC);
    SenderInformation info(1000, packageCount, 2048);
    void* package = handler.createSenderReportPackage(header, info, {});

    //read package
    RTCPHeader readHeader(0);
    SenderInformation readInfo(0,0,0);
    std::vector<ReceptionReport> reports = handler.readSenderReport(package, RTCPPackageHandler::getRTCPPackageLength(header.length), readHeader, readInfo);

    //tests
    TEST_ASSERT_MSG(handler.isRTCPPackage(package, handler.getRTCPPackageLength(header.length)), "Sender Report Package not recognized");
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_SENDER_REPORT, header.packageType);
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_SENDER_REPORT, readHeader.packageType);
    TEST_ASSERT_EQUALS(header.length, readHeader.length);
    TEST_ASSERT_EQUALS(testSSRC, readHeader.ssrc);
    TEST_ASSERT_EQUALS(packageCount, readInfo.packetCount);
    TEST_ASSERT_MSG(reports.empty(), "Reports were not empty");

}

void TestRTCP::testReceiverReportPackage()
{
    uint32_t packageLoss = 42;

    //create package
    RTCPHeader header(testSSRC);
    ReceptionReport info;
    info.ssrc = testSSRC;
    info.cumulativePackageLoss = packageLoss;
    ReceptionReport info2;
    info2.ssrc = testSSRC;
    info2.cumulativePackageLoss = packageLoss;
    void* package = handler.createReceiverReportPackage(header, {info, info2});

    //read package
    RTCPHeader readHeader(0);
    std::vector<ReceptionReport> readInfos = handler.readReceiverReport(package, handler.getRTCPPackageLength(header.length), readHeader);

    //tests
    TEST_ASSERT_MSG(handler.isRTCPPackage(package, handler.getRTCPPackageLength(header.length)), "Receiver Report Package not recognized");
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_RECEIVER_REPORT, header.packageType);
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_RECEIVER_REPORT, readHeader.packageType);
    TEST_ASSERT_EQUALS(header.length, readHeader.length);
    TEST_ASSERT_EQUALS(testSSRC, readHeader.ssrc);
    TEST_ASSERT_EQUALS(2, readInfos.size());
    TEST_ASSERT_EQUALS(packageLoss, readInfos[0].cumulativePackageLoss);
    TEST_ASSERT_EQUALS(packageLoss, readInfos[1].cumulativePackageLoss);
}

void TestRTCP::testSourceDescriptionPacakge()
{
    std::string name = "John Doe";
    std::string tool = "OHMComm";

    //create package
    RTCPHeader header(testSSRC);
    SourceDescription descr1;
    descr1.type = RTCP_SOURCE_NAME;
    descr1.value = name;
    SourceDescription descr2;
    descr2.type = RTCP_SOURCE_TOOL;
    descr2.value = tool;
    void* package = handler.createSourceDescriptionPackage(header, {descr1, descr2});

    //read package
    RTCPHeader readHeader(0);
    std::vector<SourceDescription> readDescriptions = handler.readSourceDescription(package, handler.getRTCPPackageLength(header.length),readHeader);

    //tests
    TEST_ASSERT_MSG(handler.isRTCPPackage(package, handler.getRTCPPackageLength(header.length)), "Source Description Package not recognized");
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_SOURCE_DESCRIPTION, header.packageType);
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_SOURCE_DESCRIPTION, readHeader.packageType);
    TEST_ASSERT_EQUALS(header.length, readHeader.length);
    TEST_ASSERT_EQUALS(testSSRC, readHeader.ssrc);
    TEST_ASSERT_EQUALS(2, readDescriptions.size());
    TEST_ASSERT_EQUALS(RTCP_SOURCE_NAME, readDescriptions[0].type);
    TEST_ASSERT_EQUALS(name, readDescriptions[0].value);
    TEST_ASSERT_EQUALS(RTCP_SOURCE_TOOL, readDescriptions[1].type);
    TEST_ASSERT_EQUALS(tool, readDescriptions[1].value);
}

void TestRTCP::testByePackage()
{
    std::string testMessage("Test massage");

    //create package
    RTCPHeader header(testSSRC);
    void* package = handler.createByePackage(header, testMessage);

    //read package
    RTCPHeader readHeader(0);
    std::string readMessage = handler.readByeMessage(package, handler.getRTCPPackageLength(header.length)+100, readHeader);

    //tests
    TEST_ASSERT_MSG(handler.isRTCPPackage(package, handler.getRTCPPackageLength(header.length)), "Bye Package not recognized");
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_GOODBYE, header.packageType);
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_GOODBYE, readHeader.packageType);
    TEST_ASSERT_EQUALS(header.length, readHeader.length);
    TEST_ASSERT_EQUALS(testSSRC, readHeader.ssrc);
    TEST_ASSERT_EQUALS(testMessage, readMessage);
}

void TestRTCP::testAppDefinedPackage()
{
    const char* name = (const char*)"TEXT";
    std::string someText("This is some text padded to 32 bit!!");
    unsigned int someType = 9;

    //create package
    RTCPHeader header(testSSRC);
    ApplicationDefined appDefined(name, someText.size(), (char*)someText.data(), someType);
    void* package = handler.createApplicationDefinedPackage(header, appDefined);

    //read package
    RTCPHeader readHeader(0);
    ApplicationDefined readAppDefined = handler.readApplicationDefinedMessage(package, handler.getRTCPPackageLength(header.length), readHeader);

    //tests
    TEST_ASSERT_MSG(handler.isRTCPPackage(package, handler.getRTCPPackageLength(header.length)), "Application Defined Package not recognized");
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_APPLICATION_DEFINED, header.packageType);
    TEST_ASSERT_EQUALS(RTCP_PACKAGE_APPLICATION_DEFINED, readHeader.packageType);
    TEST_ASSERT_EQUALS(header.length, readHeader.length);
    TEST_ASSERT_EQUALS(testSSRC, readHeader.ssrc);
    TEST_ASSERT_EQUALS(someText, std::string(readAppDefined.data, readAppDefined.dataLength));
    TEST_ASSERT_EQUALS(someType, readAppDefined.subType);
}

void TestRTCP::testIsRTCPPackage()
{
    //positive test
    RTCPHeader rtcpHeader(testSSRC);
    void* rtcpPackage = handler.createByePackage(rtcpHeader, "Goodbye!");
    //negative test
    RTPPackageHandler h(2048);
    void* someData = (void*)(std::string("Some Data!").data());
    const void* rtpPackage = h.createNewRTPPackage(someData, 10);

    //tests
    TEST_ASSERT(true == handler.isRTCPPackage(rtcpPackage, handler.getRTCPPackageLength(rtcpHeader.length)));
    TEST_ASSERT(false == handler.isRTCPPackage(rtpPackage, h.getActualPayloadSize() + h.getRTPHeaderSize()));
}





