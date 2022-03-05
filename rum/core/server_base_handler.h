/*
 * Created by Ivan B on 2022/2/6.
 */

#ifndef RUM_CORE_SERVER_BASE_HANDLER_H_
#define RUM_CORE_SERVER_BASE_HANDLER_H_

namespace rum{

class ServerBaseImpl;
class NodeBase;

class ServerBaseHandler {
  private:
    friend NodeBase;
    ServerBaseImpl* pimpl_ = nullptr;

  protected:
    explicit ServerBaseHandler(ServerBaseImpl* pimpl);

  public:
    inline virtual ~ServerBaseHandler() = default;
};

}
#endif //RUM_CORE_SERVER_BASE_HANDLER_H_
