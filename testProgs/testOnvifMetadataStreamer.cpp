#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <chrono>
#include <liveMedia.hh>
#include <thread>

class TextSource : public FramedSource {
public:
  static TextSource *createNew(UsageEnvironment &env) {
    return new TextSource(env);
  }

protected:
  TextSource(UsageEnvironment &env) : FramedSource(env), m_last_pts() {}

  void doGetNextFrame() override {
    if (!isCurrentlyAwaitingData()) {
      return;
    }

    const char *buff =
        R"(<?xml version=\"1.0\" encoding=\"utf-8\"?>
<tt:MetadataStream
    xmlns:tt=\"http://www.onvif.org/ver10/schema\">
    <tt:Extension>
        <NavigationalData version=\"1.0\">
            <Latitude>55.861974</Latitude>
            <Longitude>-4.258526</Longitude>
        </NavigationalData>
    </tt:Extension>
</tt:MetadataStream>)";

    auto buff_len = strlen(buff);

    if (buff_len <= fMaxSize) {
      fNumTruncatedBytes = 0;
      fFrameSize = buff_len;
      memcpy(fTo, buff, buff_len);

      int64_t ts = std::chrono::system_clock::now().time_since_epoch().count();
      if (m_last_pts == 0)
        m_last_pts = ts;

      fPresentationTime.tv_sec = (long)ts / 1000;
      ts -= fPresentationTime.tv_sec * 1000;
      fPresentationTime.tv_usec = (long)ts * 1000;

      m_last_pts = ts;

      std::this_thread::sleep_for(std::chrono::seconds(1));
      FramedSource::afterGetting(this);
    }
  }

private:
  int m_last_pts;
};

class OnvifSink : public TextRTPSink {
public:
  static OnvifSink *createNew(UsageEnvironment &env, Groupsock *RTPgs,
                              unsigned char rtpPayloadFormat) {
    return new OnvifSink(env, RTPgs, rtpPayloadFormat);
  }

  ~OnvifSink() override {
    stopPlaying(); // call this now, because we won't have our 'idle filter'
                   // when the base class destructor calls it // later.
    fSource = NULL; // for the base class destructor, which gets called next
  }

protected:
  OnvifSink(UsageEnvironment &env, Groupsock *RTPgs,
            unsigned char rtpPayloadFormat)
      : TextRTPSink(env, RTPgs, rtpPayloadFormat, 1000, "VND.ONVIF.METADATA") {}

  void doSpecialFrameHandling(unsigned int fragmentationOffset,
                              unsigned char *frameStart,
                              unsigned int numBytesInFrame,
                              struct timeval framePresentationTime,
                              unsigned int numRemainingBytes) override {
    TextRTPSink::doSpecialFrameHandling(fragmentationOffset, frameStart,
                                        numBytesInFrame, framePresentationTime,
                                        numRemainingBytes);
    setMarkerBit(); // 'M' bit set for Onvif
  }

  Boolean
  frameCanAppearAfterPacketStart(unsigned char const * /*frameStart*/,
                                 unsigned /*numBytesInFrame*/) const override {
    return False; // We don't concatenate input data; instead, send it out
                  // immediately
  }
};

struct RTSPServerController {
  UsageEnvironment *env;
  char const *inputFileName = "test.264";
  TextSource *xmlSource;
  RTPSink *textSink;

  void Setup() { // Begin by setting up our usage environment:
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);

    // Create 'groupsocks' for RTP and RTCP:
    // Note: This is a multicast address.  If you wish instead to stream
    // using unicast, then you should use the "testOnDemandRTSPServer"
    // test program - not this test program - as a model.
    struct in_addr destinationAddress = {};
    destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);

    const unsigned short rtpPortNum = 18888;
    const unsigned short rtcpPortNum = rtpPortNum + 1;
    const unsigned char ttl = 255;

    const Port rtpPort(rtpPortNum);
    const Port rtcpPort(rtcpPortNum);

    auto *rtpGroupsock = new Groupsock(*env, destinationAddress, rtpPort, ttl);
    rtpGroupsock->multicastSendOnly(); // we're a SSM source
    auto *rtcpGroupsock =
        new Groupsock(*env, destinationAddress, rtcpPort, ttl);
    rtcpGroupsock->multicastSendOnly(); // we're a SSM source

    // Create a 'T140 Text RTP' sink from the RTP 'groupsock':
    OutPacketBuffer::maxSize = 100000;
    textSink = OnvifSink::createNew(*env, rtpGroupsock, 96);

    // Create (and start) a 'RTCP instance' for this RTP sink:
    const unsigned estimatedSessionBandwidth =
        500; // in kbps; for RTCP b/w share
    const unsigned maxCNAMElen = 100;
    unsigned char CNAME[maxCNAMElen + 1];
    gethostname((char *)CNAME, maxCNAMElen);
    CNAME[maxCNAMElen] = '\0'; // just in case
    RTCPInstance *rtcp = RTCPInstance::createNew(
        *env, rtcpGroupsock, estimatedSessionBandwidth, CNAME, textSink,
        nullptr /* we're a server */, True /* we're a SSM source */);
    // Note: This starts RTCP running automatically

    RTSPServer *rtspServer = RTSPServer::createNew(*env, 8554);
    if (rtspServer == nullptr) {
      *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
      exit(1);
    }
    ServerMediaSession *sms = ServerMediaSession::createNew(
        *env, "testStream", inputFileName,
        "Session streamed by \"testH264VideoStreamer\"", True /*SSM*/);
    sms->addSubsession(
        PassiveServerMediaSubsession::createNew(*textSink, rtcp));
    rtspServer->addServerMediaSession(sms);

    char *url = rtspServer->rtspURL(sms);
    *env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
  }

  void play() {
    // Open the input file as a 'byte-stream file xmlSource':
    xmlSource = TextSource::createNew(*env);
    if (xmlSource == nullptr) {
      *env << "Unable to open file \"" << inputFileName
           << "\" as a byte-stream file xmlSource\n";
      exit(1);
    }

    // Finally, start playing:
    *env << "Beginning to read from file...\n";
    textSink->startPlaying(*xmlSource, afterPlaying, this);
  }

  static void afterPlaying(void *clientData) {
    auto p_controller = reinterpret_cast<RTSPServerController *>(clientData);

    *p_controller->env << "...done reading from file\n";
    p_controller->textSink->stopPlaying();
    Medium::close(p_controller->xmlSource);
    // Note that this also closes the input file that this source read from.

    // Start playing once again:
    p_controller->play();
  }
};

int main(int argc, char **argv) {
  RTSPServerController controller;
  controller.Setup();

  // Start the streaming:
  *controller.env << "Beginning streaming...\n";
  controller.play();

  controller.env->taskScheduler().doEventLoop(); // does not return
  return 0; // only to prevent compiler warning
}
