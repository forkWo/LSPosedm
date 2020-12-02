
#pragma once

#include <vector>
#include <string>
#include "JNIHelper.h"
#include <utility>
#include <art/runtime/native/native_util.h>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include "config.h"
#include "utils.h"

namespace edxp {


    class ConfigManager {
    private:
        inline static const auto kPrimaryInstallerPkgName = "org.meowcat.edxposed.manager"_str;
        inline static const auto kXposedPropName = "edconfig.jar"_str;
        inline static const std::vector<std::string> kXposedInjectDexPath = {
                "edxp.dex",
                "eddalvikdx.dex",
                "eddexmaker.dex",
        };

    public:
        inline static ConfigManager *GetInstance() {
            return instances_[current_user].get();
        }

        inline auto IsInitialized() const { return initialized_; }

        inline static void SetCurrentUser(uid_t user) {
            if (auto instance = instances_.find(user);
                    instance == instances_.end() ||  !instance->second) {
                instances_[user] = std::make_unique<ConfigManager>(user);
            } else if(instance->second->NeedUpdateConfig()) {
                instances_[user] = std::make_unique<ConfigManager>(user, instance->second->IsInitialized());
            }
        }

        inline static auto ReleaseInstances() {
            return std::move(instances_);
        }

        // Always true now
        inline auto IsBlackWhiteListEnabled() const { return true; }

        inline auto IsResourcesHookEnabled() const { return resources_hook_enabled_; }

        inline auto IsDeoptBootImageEnabled() const { return deopt_boot_image_enabled_; }

        inline auto IsNoModuleLogEnabled() const { return no_module_log_enabled_; }

        inline auto GetInstallerPackageName() const { return installer_pkg_name_; }

        inline auto GetLibSandHookName() const { return kLibSandHookName; }

        inline auto GetDataPathPrefix() const { return data_path_prefix_; }

        inline static auto GetFrameworkPath(const std::string &suffix = {}) {
            return GetMiscPath() / "framework" / suffix;
        }

        inline auto GetXposedPropPath() const { return GetFrameworkPath(kXposedPropName); }

        inline auto GetConfigPath(const std::string &suffix = {}) const {
            return base_config_path_ / "conf" / suffix;
        }

        inline auto GetLogPath(const std::string &suffix = {}) const {
            return base_config_path_ / "log" / suffix;
        }

        inline auto GetBaseConfigPath() const { return base_config_path_; }

        inline auto GetPrefsPath(const std::string &pkg_name) const {
            return base_config_path_ / "prefs" / pkg_name;
        }

        std::vector<std::string> GetAppModuleList(const std::string &pkg_name) const;

        bool IsAppNeedHook(const std::string &pkg_name) const;

        bool NeedUpdateConfig() const {
            return last_write_time_ < GetLastWriteTime();
        }

        void EnsurePermission(const std::string &pkg_name, uid_t uid) const;


    private:
        inline static std::unordered_map<uid_t, std::unique_ptr<ConfigManager>> instances_{};
        inline static uid_t current_user = 0u;
        inline static std::filesystem::path misc_path_;
        inline static std::vector<std::filesystem::path> inject_dex_paths_;
        inline static const bool use_prot_storage_ = GetAndroidApiLevel() >= __ANDROID_API_N__;

        static decltype(misc_path_) GetMiscPath();

        const uid_t user_;
        const std::filesystem::path data_path_prefix_;
        const std::filesystem::path base_config_path_;
        const bool initialized_ = false;
        const std::filesystem::path installer_pkg_name_;
        const bool white_list_enable_ = false;
        const bool deopt_boot_image_enabled_ = false;
        const bool no_module_log_enabled_ = false;
        const bool resources_hook_enabled_ = false;
        // snapshot at boot
        const std::unordered_set<std::string> white_list_;
        const std::unordered_set<std::string> black_list_;

        const std::unordered_map<std::string, std::pair<std::string, std::unordered_set<std::string>>> modules_list_;

        const std::filesystem::file_time_type last_write_time_;

        ConfigManager(uid_t uid, bool initialized=false);

        static std::unordered_set<std::string> GetAppList(const std::filesystem::path &dir);

        std::string RetrieveInstallerPkgName() const;

        static std::string GetPackageNameFromBaseApkPath(const std::filesystem::path &path);

        std::remove_const_t<decltype(modules_list_)> GetModuleList();

        std::filesystem::file_time_type GetLastWriteTime() const;

        bool InitConfigPath() const;

        friend std::unique_ptr<ConfigManager> std::make_unique<ConfigManager>(uid_t &);
        friend std::unique_ptr<ConfigManager> std::make_unique<ConfigManager>(uid_t &, bool&&);

        std::filesystem::path RetrieveBaseConfigPath() const;

    public:
        static decltype(inject_dex_paths_) GetInjectDexPaths();
    };

} // namespace edxp

