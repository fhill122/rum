/*
 * Created by Ivan B on 2021/12/21.
 */

#ifndef RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_
#define RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_

#include "rum/cpprum/serialization/serializer.h"
#include "autoserialize.h"

namespace rum{

/**
 * Native struct serialization.
 * Limitation:
 *  - machine dependent, may not work on devices over network that have different arch/system
 */
class SerializerNative : public Serializer<SerializerNative>{

  public:
    template <class T>
    std::unique_ptr<Message>
    serialize(const std::shared_ptr<const T> &object) const {
        size_t size = AutoGetSerializationSize(*object);
        auto msg = std::make_unique<Message>(size);
        AutoSerialize((char*)msg->data(), *object);
        return msg;
    }

    template<typename T>
    std::unique_ptr<T> deserialize(std::shared_ptr<const Message> &msg_in,
                                   const std::string &msg_protocol="") const{
        if (msg_protocol!=Protocol()) return nullptr;
        // T must have default constructor
        auto t = std::make_unique<T>();
        AutoDeserialize((char*)msg_in->data(), *t);
        return t;
    }

    inline static std::string Protocol(){
        return "native";
    }

    template<typename T, typename... Args>
    static bool SerializeToFile(const std::string &path, const T &t, const Args &... args) {
        using namespace std;
        ofstream file(path, ios::out | ios::binary);
        if (!file){
            log.e(__func__, "file failed to open: %s", path.c_str());
            return false;
        }

        size_t size = AutoGetSerializationSize(t, args...);
        vector<char> buffer(size);
        AutoSerialize(buffer.data(), t, args...);

        file.write(buffer.data(), buffer.size());
        return true;
    }

    template<typename T, typename... Args>
    static bool DeserializeFromFile(const std::string &path, T &t, Args &... args) {
        using namespace std;
        ifstream file(path, ios::in | ios::binary);
        if (!file) {
            log.e(__func__, "file failed to open: %s", path.c_str());
            return false;
        }

        file.seekg(0, ios::end);
        vector<char> buffer(file.tellg());
        file.seekg(0, ios::beg);
        file.read(buffer.data(), buffer.size());
        if (!file){
            log.e(__func__, "file failed to read: %s", path.c_str());
            return false;
        }

        AutoDeserialize(buffer.data(), t, args...);
        return true;
    }
};

}
#endif //RUM_SERIALIZATION_NATIVE_SERIALIZER_NATIVE_H_
