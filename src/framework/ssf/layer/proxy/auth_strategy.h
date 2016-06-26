#ifndef SSF_LAYER_PROXY_AUTH_STRATEGY_H_
#define SSF_LAYER_PROXY_AUTH_STRATEGY_H_

#include "ssf/layer/proxy/http_connect_request.h"
#include "ssf/layer/proxy/http_response.h"
#include "ssf/layer/proxy/proxy_endpoint_context.h"

namespace ssf {
namespace layer {
namespace proxy {
namespace detail {

class AuthStrategy {
 public:
  enum Status : int {
    kAuthenticationFailure = -1,
    kAuthenticating = 0,
    kAuthenticated = 1
  };

 public:
  virtual ~AuthStrategy() {}

  virtual bool Support(const HttpResponse& response) const = 0;

  virtual void ProcessResponse(const HttpResponse& response) = 0;

  virtual void PopulateRequest(const ProxyEndpointContext& proxy_ep_ctx,
                               HttpConnectRequest* p_request) = 0;

  inline Status status() const { return status_; }

 protected:
  AuthStrategy(Status status) : status_(status) {}

 protected:
  Status status_;
};

}  // detail
}  // proxy
}  // layer
}  // ssf

#endif  // SSF_LAYER_PROXY_AUTH_STRATEGY_H_