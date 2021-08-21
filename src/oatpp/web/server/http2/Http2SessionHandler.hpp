/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Benedikt-Alexander Mokroß <github@bamkrs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef oatpp_web_server_http2_Http2SessionHandler_hpp
#define oatpp_web_server_http2_Http2SessionHandler_hpp

#include "oatpp/web/server/HttpRouter.hpp"

#include "oatpp/web/server/interceptor/RequestInterceptor.hpp"
#include "oatpp/web/server/interceptor/ResponseInterceptor.hpp"
#include "oatpp/web/server/handler/ErrorHandler.hpp"

#include "oatpp/web/protocol/http/encoding/ProviderCollection.hpp"

#include "oatpp/web/protocol/http/incoming/RequestHeadersReader.hpp"
#include "oatpp/web/protocol/http/incoming/Request.hpp"

#include "oatpp/web/protocol/http/outgoing/Response.hpp"
#include "oatpp/web/protocol/http/utils/CommunicationUtils.hpp"

#include "oatpp/core/data/stream/StreamBufferedProxy.hpp"
#include "oatpp/core/async/Processor.hpp"

#include "oatpp/web/protocol/http2/Http2.hpp"
#include "oatpp/web/server/http2/Http2StreamHandler.hpp"
#include "oatpp/web/server/http2/Http2ProcessingComponents.hpp"
#include "oatpp/web/server/http2/PriorityStreamScheduler.hpp"
#include "oatpp/web/server/http2/Http2Settings.hpp"

#include "oatpp/core/async/Coroutine.hpp"

namespace oatpp { namespace web { namespace server { namespace http2 {

/**
 * HttpProcessor. Helper class to handle HTTP processing.
 */
class Http2SessionHandler : public oatpp::async::Coroutine<Http2SessionHandler> {
 public:
  typedef web::protocol::http::incoming::RequestHeadersReader RequestHeadersReader;
  typedef protocol::http::utils::CommunicationUtils::ConnectionState ConnectionState;
  typedef protocol::http2::Frame::Header::FrameType FrameType;
  typedef protocol::http2::Frame::Header FrameHeader;
  typedef protocol::http2::error::ErrorCode H2ErrorCode;

 private:
  static const char* TAG;

  struct ProcessingResources {

    ProcessingResources(const std::shared_ptr<processing::Components>& pComponents,
                        const std::shared_ptr<http2::Http2Settings>& pInSettings,
                        const std::shared_ptr<http2::Http2Settings>& pOutSettings,
                        const std::shared_ptr<oatpp::data::stream::InputStreamBufferedProxy>& pInStream,
                        const std::shared_ptr<http2::PriorityStreamSchedulerAsync>& pOutStream);
    static std::shared_ptr<ProcessingResources> createShared(const std::shared_ptr<processing::Components>& pComponents,
                                                             const std::shared_ptr<http2::Http2Settings>& pInSettings,
                                                             const std::shared_ptr<http2::Http2Settings>& pOutSettings,
                                                             const std::shared_ptr<oatpp::data::stream::InputStreamBufferedProxy>& pInStream,
                                                             const std::shared_ptr<http2::PriorityStreamSchedulerAsync>& pOutStream) {
      return std::make_shared<ProcessingResources>(pComponents, pInSettings, pOutSettings, pInStream, pOutStream);
    }
    ~ProcessingResources();

    std::shared_ptr<processing::Components> components;
    std::shared_ptr<oatpp::data::stream::IOStream> connection;
    std::shared_ptr<oatpp::data::stream::InputStreamBufferedProxy> inStream;
    std::shared_ptr<http2::PriorityStreamSchedulerAsync> outStream;

    /**
     * Collection of all streams in an ordered map
     */
    std::map<v_uint32, std::shared_ptr<Http2StreamHandler>> h2streams;
    std::shared_ptr<Http2StreamHandler> lastStream;
    v_uint32 highestNonIdleStreamId;

    /**
     * For now here, should be provided with a kind of factory via components
     */
    std::shared_ptr<web::protocol::http2::hpack::Hpack> hpack;


    std::shared_ptr<http2::Http2Settings> inSettings;
    std::shared_ptr<http2::Http2Settings> outSettings;

    protocol::http2::error::ErrorCode error;
  };

  template<class F>
  class SendFrameCoroutine : public async::Coroutine<F> {
   public:
    virtual ~SendFrameCoroutine() = default;
    virtual v_uint32 framePriority() const {return PriorityStreamSchedulerAsync::PRIORITY_MAX;}
    virtual const data::share::MemoryLabel* frameData() const {return nullptr;};
    virtual PriorityStreamSchedulerAsync *scheduler() const const = 0;
    virtual v_uint8 frameFlags() const {return 0;}
    virtual FrameType frameType() const = 0;
    virtual v_uint32 frameStreamId() const {return 0;}
   private:
    FrameHeader m_header;

   public:
    SendFrameCoroutine() : m_header(frameData() ? frameData()->getSize() : 0, frameFlags(), frameType(), frameStreamId()) {}

    Action act() override {
      return scheduler()->lock(framePriority(), async::Coroutine<F>::yieldTo(&SendFrameCoroutine<F>::sendFrameHeader));
    }

    Action sendFrameHeader() {
      OATPP_LOGD(TAG, "Sending %s (length:%lu, flags:0x%02x, streamId:%lu)", FrameHeader::frameTypeStringRepresentation(m_header.getType()), m_header.getLength(), m_header.getFlags(), m_header.getStreamId());
      v_uint8 buf[9];
      m_header.writeToBuffer(buf);
      data::buffer::InlineWriteData data(buf, 9);
      if (frameData() && frameData()->getSize() > 0) {
        return scheduler()->writeExactSizeDataAsyncInline(data, async::Coroutine<F>::yieldTo(&SendFrameCoroutine<F>::sendFrameData));
      }
      return scheduler()->writeExactSizeDataAsyncInline(data, async::Coroutine<F>::yieldTo(&SendFrameCoroutine<F>::finalize));
    }

    Action sendFrameData() {
      data::buffer::InlineWriteData data(frameData()->getData(), frameData()->getSize());
      return scheduler()->writeExactSizeDataAsyncInline(data, async::Coroutine<F>::yieldTo(&SendFrameCoroutine<F>::finalize));
    }

    Action finalize() {
      scheduler()->unlock();
      return async::Coroutine<F>::finish();
    }

  };

  std::shared_ptr<ProcessingResources> m_resources;
  std::shared_ptr<processing::Components> m_components;
  std::shared_ptr<oatpp::data::stream::IOStream> m_connection;
  std::atomic_long *m_counter;


 public:

    /**
     * Constructor.
     * @param components - &l:HttpProcessor::Components;.
     * @param connection - &id:oatpp::data::stream::IOStream;.
     * @param taskCounter - Counter to increment for every creates task
     */
    Http2SessionHandler(const std::shared_ptr<processing::Components>& components,
         const std::shared_ptr<oatpp::data::stream::IOStream>& connection,
         std::atomic_long *taskCounter);

    /**
     * Constructor.
     * @param components - &l:HttpProcessor::Components;.
     * @param connection - &id:oatpp::data::stream::IOStream;.
     * @param taskCounter - Counter to increment for every creates task
     * @param delegationParameters - Parameter-Map from delegation (if any)
     */
    Http2SessionHandler(const std::shared_ptr<processing::Components>& components,
         const std::shared_ptr<oatpp::data::stream::IOStream>& connection,
         std::atomic_long *taskCounter,
         const std::shared_ptr<const network::ConnectionHandler::ParameterMap> &delegationParameters);

    /**
     * Copy-Constructor to correctly count sessions.
     */
    Http2SessionHandler(const Http2SessionHandler &copy);

    /**
     * Copy-Assignment to correctly count sessions.
     * @param t - Task to copy
     * @return
     */
    Http2SessionHandler &operator=(const Http2SessionHandler &t);

    /**
     * Move-Constructor to correclty count sessions;
     */
    Http2SessionHandler(Http2SessionHandler &&move);

     /**
      * Move-Assignment to correctly count tasks.
      * @param t
      * @return
      */
     Http2SessionHandler &operator=(Http2SessionHandler &&t);

    /**
     * Destructor, needed for counting.
     */
    ~Http2SessionHandler() override;

 public:
  Action act() override;

 private:
  Action nextRequest();
  Action handleFrame(const std::shared_ptr<FrameHeader> &header);
  Action connectionError(H2ErrorCode errorCode);
  Action connectionError(H2ErrorCode errorCode, const std::string &message);

  Action teardown();

  static void stop(ProcessingResources& resources);

  Action delegateToHandler(const std::shared_ptr<Http2StreamHandler> &handler,
                                           const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                           Http2SessionHandler::ProcessingResources &resources,
                                           protocol::http2::Frame::Header &header);
  static std::shared_ptr<Http2StreamHandler> findOrCreateStream(v_uint32 ident,
                                                                ProcessingResources &resources);
  async::Action consumeStream(v_io_size streamPayloadLength, async::Action &&next);

  Action sendSettingsFrame();
  Action ackSettingsFrame();
  Action answerPingFrame();
  Action sendGoawayFrame(v_uint32 lastStream, H2ErrorCode errorCode);
  Action sendResetStreamFrame(v_uint32 stream, H2ErrorCode errorCode);

  };

}}}}

#endif /* oatpp_web_server_http2_Http2SessionHandler_hpp */
