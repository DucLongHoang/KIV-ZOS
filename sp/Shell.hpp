#pragma once

#include <memory>
#include <sstream>
#include <functional>
#include <unordered_map>
#include "FAT.hpp"

/**
 * Class Shell
 */
class Shell {

	private:
        std::string mFsName;
        std::string mCWD;
		std::unique_ptr<IFilesystem> mFilesystem;

	public:
		enum class CMD {
			CP, MV, RM, MKDIR, RMDIR, LS, CAT, CD, PWD,
			INFO, INCP, OUTCP, LOAD, FORMAT, XCP, EXIT, UNKNOWN
		};

		explicit Shell(const std::string& fsName);
		~Shell() = default;

        void mount_fs(const std::string& fsName);

        [[noreturn]] void run();
		void process_input(const std::string& input);
		void execute_cmd(const CMD& CMD, const std::vector<std::string>& args);
        bool check_argc(const std::vector<std::string>& args, uint needed);

		// shell commands
		bool cp(const std::vector<std::string>& args);
		bool mv(const std::vector<std::string>& args);
		bool rm(const std::vector<std::string>& args);
		bool mkdir(const std::vector<std::string>& args);
		bool rmdir(const std::vector<std::string>& args);
		bool ls(const std::vector<std::string>& args);
		bool cat(const std::vector<std::string>& args);
		bool cd(const std::vector<std::string>& args);
		bool pwd(const std::vector<std::string>& args);
		bool info(const std::vector<std::string>& args);
		bool incp(const std::vector<std::string>& args);
		bool outcp(const std::vector<std::string>& args);
		bool load(const std::vector<std::string>& args);
		bool format(const std::vector<std::string>& args);
		bool xcp(const std::vector<std::string>& args);
		bool exit(const std::vector<std::string>& args);
		bool unknown(const std::vector<std::string>& args);

		// map of commands
		const std::unordered_map<std::string, CMD> cmdMap = {
                {"cp"s, CMD::CP}, {"mv"s, CMD::MV}, {"rm"s, CMD::RM}, {"mkdir"s, CMD::MKDIR},
                {"rmdir", CMD::RMDIR}, {"ls"s, CMD::LS}, {"cat"s, CMD::CAT}, {"cd"s, CMD::CD},
                {"pwd"s, CMD::PWD}, {"info"s, CMD::INFO}, {"incp"s, CMD::INCP}, {"outcp"s, CMD::OUTCP},
                {"load"s, CMD::LOAD}, {"format"s, CMD::FORMAT},{"xcp"s, CMD::XCP}, {"exit"s, CMD::EXIT}
        };
};
