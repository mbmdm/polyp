#ifndef INSTANCE_H
#define INSTANCE_H

#define VK_NO_PROTOTYPES
#include "dispatch_table.h"
#include "vk_destroyer.h"

namespace polyp {
namespace engine {

class Instance final {
public:
    Instance();
    explicit Instance(const char* appName);
    Instance(const char* appName, uint32_t major, uint32_t minor, uint32_t patch);
    Instance(const char* appName, uint32_t major, uint32_t minor, uint32_t patch, 
             const std::vector<const char*>& desiredExt);

    [[nodiscard]] std::string getAppName() const;
    [[nodiscard]] std::tuple<uint32_t, uint32_t, uint32_t> getAppVersion() const;
    [[nodiscard]] std::vector<const char*> getExtensions() const;
    [[nodiscard]] DispatchTable getDispatchTable() const;

    [[nodiscard]] bool init();

private:

    uint32_t mMajorVersion;
    uint32_t mMinorVersion;
    uint32_t mPatchVersion;

    std::string mAppicationName;

    VkDestroyer(VkLibrary) mLibrary;
    VkDestroyer(VkInstance) mHandle;

    std::vector<const char*> mExtensions;

    DispatchTable mDispTable;
};

} // engine
} // polyp

#endif // INSTANCE_H
