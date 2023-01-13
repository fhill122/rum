/*
 * Created by Ivan B on 2022/8/23.
 */

#ifndef RUM_COMMON_SERIALIZATION_H_
#define RUM_COMMON_SERIALIZATION_H_

#include <memory>

#include "message.h"

namespace rum{
// todo ivan. allow other version of SubFunc and SrvFunc, e.g. taking reference

template<typename T>
using SubFunc = std::function<void(const std::shared_ptr<const T>&)>;
// void is actually the type that DeserFunc converts to
using InterProcFunc = std::function<void(const std::shared_ptr<const void>&)>;
// void is actually the type of scheduled intra-proc object
using IntraProcFunc = std::function<void(const std::shared_ptr<const void>&)>;

template<typename T=void>
using DeserFunc = std::function<
std::shared_ptr<const T> (std::shared_ptr<const Message>&, const std::string&) >;

template<typename T=void>
using SerFunc = std::function<
std::unique_ptr<Message> (const std::shared_ptr<const T>&) >;

template<typename T>
using IntraProcFactoryFunc = std::function<std::shared_ptr<const T>(const std::shared_ptr<const void>&)>;

// SubT, PubT
template<typename Q, typename P>
using SrvFunc = std::function<bool(const std::shared_ptr<const Q>& request, std::shared_ptr<P>& response)>;

using SrvIntraProcFunc = std::function<bool(std::shared_ptr<const void>& request,
                                            std::shared_ptr<void>& response)>;

using SrvInterProcFunc = std::function<bool(std::shared_ptr<const Message>& request,
                                            const std::string& req_protocol, std::shared_ptr<Message>& response) >;

}

#endif //RUM_COMMON_SERIALIZATION_H_
