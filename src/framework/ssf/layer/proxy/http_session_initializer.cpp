#include <boost/format.hpp>

#include "ssf/error/error.h"
#include "ssf/layer/proxy/http_session_initializer.h"
#include "ssf/layer/proxy/basic_auth_strategy.h"
#include "ssf/log/log.h"

namespace ssf {
namespace layer {
namespace proxy {
namespace detail {

HttpSessionInitializer::HttpSessionInitializer()
    : status_(Status::kContinue),
      target_host_(""),
      target_port_(""),
      proxy_ep_ctx_(),
      auth_strategies_(),
      p_current_auth_strategy_(nullptr) {}

void HttpSessionInitializer::Reset(const std::string& target_host,
                                   const std::string& target_port,
                                   const ProxyEndpointContext& proxy_ep_ctx,
                                   boost::system::error_code& ec) {
  status_ = Status::kContinue;
  target_host_ = target_host;
  target_port_ = target_port;
  proxy_ep_ctx_ = proxy_ep_ctx;

  auth_strategies_.clear();
  auth_strategies_.emplace_back(new detail::BasicAuthStrategy());
}

std::string HttpSessionInitializer::GenerateRequest(
    boost::system::error_code& ec) {
  if (status_ != Status::kContinue) {
    ec.assign(ssf::error::interrupted, ssf::error::get_ssf_category());
    return "";
  }

  HttpConnectRequest request(target_host_, target_port_);

  if (p_current_auth_strategy_ != nullptr) {
    p_current_auth_strategy_->PopulateRequest(proxy_ep_ctx_, &request);
  }

  return request.GenerateRequest();
}

void HttpSessionInitializer::ProcessResponse(const HttpResponse& response,
                                             boost::system::error_code& ec) {
  if (response.Success()) {
    status_ = Status::kSuccess;
    return;
  }

  if (!response.AuthenticationRequired()) {
    SSF_LOG(kLogError) << "network[proxy]: unexpected response type";
    status_ = Status::kError;
    return;
  }

  // find auth strategy
  if (p_current_auth_strategy_ == nullptr ||
      p_current_auth_strategy_->status() ==
          AuthStrategy::kAuthenticationFailure) {
    p_current_auth_strategy_ = nullptr;
    for (auto& p_auth_strategy : auth_strategies_) {
      if (p_auth_strategy->Support(response)) {
        p_current_auth_strategy_ = p_auth_strategy.get();
        break;
      }
    }
  }

  if (p_current_auth_strategy_ == nullptr) {
    SSF_LOG(kLogError) << "network[proxy]: authentication strategies failed";
    status_ = Status::kError;
    return;
  }

  p_current_auth_strategy_->ProcessResponse(response);
  switch (p_current_auth_strategy_->status()) {
    case AuthStrategy::Status::kAuthenticating:
    case AuthStrategy::Status::kAuthenticationFailure:
      break;
    case AuthStrategy::Status::kAuthenticated:
      status_ = Status::kSuccess;
      break;
    default:
      status_ = Status::kError;
      break;
  }
}

}  // detail
}  // proxy
}  // layer
}  // ssf