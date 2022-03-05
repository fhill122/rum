/*
 * Created by Ivan B on 2022/2/6.
 */

#ifndef RUM_CPPRUM_SERVER_H_
#define RUM_CPPRUM_SERVER_H_

#include "rum/core/server_base_handler.h"
#include "rum/serialization/serializer.h"

namespace rum{

// would this better than inheritance here?
class Server{
  public:
    using SharedPtr = std::shared_ptr<Server>;
    using UniquePtr = std::unique_ptr<Server>;

  private:
    ServerBaseHandler handler_;

  public:
    explicit Server(ServerBaseHandler&& handler) : handler_(handler){}

};

}

#endif //RUM_CPPRUM_SERVER_H_
