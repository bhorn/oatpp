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

#include "Http2StreamHandler.hpp"

namespace oatpp { namespace web { namespace server { namespace http2 {

Http2StreamHandler::ConnectionState Http2StreamHandler::handleData(v_uint8 flags,
                                                                   const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                   v_io_size streamPayloadLength) {
  return Http2StreamHandler::ConnectionState::CLOSING;
}

Http2StreamHandler::ConnectionState Http2StreamHandler::handleHeaders(v_uint8 flags,
                                                                      const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                      v_io_size streamPayloadLength) {
  return Http2StreamHandler::ConnectionState::CLOSING;
}

Http2StreamHandler::ConnectionState Http2StreamHandler::handlePriority(v_uint8 flags,
                                                                       const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                       v_io_size streamPayloadLength) {
  return Http2StreamHandler::ConnectionState::CLOSING;
}

Http2StreamHandler::ConnectionState Http2StreamHandler::handleResetStream(v_uint8 flags,
                                                                          const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                          v_io_size streamPayloadLength) {
  return Http2StreamHandler::ConnectionState::CLOSING;
}

Http2StreamHandler::ConnectionState Http2StreamHandler::handleSettings(v_uint8 flags,
                                                                       const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                       v_io_size streamPayloadLength) {
  return Http2StreamHandler::ConnectionState::CLOSING;
}

Http2StreamHandler::ConnectionState Http2StreamHandler::handlePushPromise(v_uint8 flags,
                                                                          const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                          v_io_size streamPayloadLength) {
  return Http2StreamHandler::ConnectionState::CLOSING;
}

Http2StreamHandler::ConnectionState Http2StreamHandler::handleGoAway(v_uint8 flags,
                                                                     const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                     v_io_size streamPayloadLength) {
  return Http2StreamHandler::ConnectionState::CLOSING;
}

Http2StreamHandler::ConnectionState Http2StreamHandler::handleWindowUpdate(v_uint8 flags,
                                                                           const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                           v_io_size streamPayloadLength) {
  return Http2StreamHandler::ConnectionState::CLOSING;
}

Http2StreamHandler::ConnectionState Http2StreamHandler::handleContinuation(v_uint8 flags,
                                                                           const std::shared_ptr<data::stream::InputStreamBufferedProxy> &stream,
                                                                           v_io_size streamPayloadLength) {
  return Http2StreamHandler::ConnectionState::CLOSING;
}

}}}}