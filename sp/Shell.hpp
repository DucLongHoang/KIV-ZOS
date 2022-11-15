#pragma once

#include <memory>
#include <sstream>
#include <functional>
#include <unordered_map>
#include "Filesystem.hpp"

/**
 *
 */
class Shell {

	private:
		std::shared_ptr<IFilesystem> fs;

	public:
		enum class CMD {
			CP, MV, RM, MKDIR, RMDIR, LS, CAT, CD, PWD, 
			INFO, INCP, OUTCP, LOAD, FORMAT, XCP, EXIT, UNKNOWN
		};

		Shell(const std::shared_ptr<IFilesystem>& ifs) { fs = ifs; }
		~Shell() = default;

		void run();
		void process_input(const std::string& input);
		void execute_cmd(const CMD& CMD, const std::vector<std::string>& args);

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

		// map of comands
		std::unordered_map<const std::string&, const CMD&> cmdMap = {
			{"cp", CMD::CP}, {"mv", CMD::MV}, {"rm", CMD::RM}, {"mkdir", CMD::MKDIR},
			{"rmdir", CMD::RMDIR}, {"ls", CMD::LS}, {"cat", CMD::CAT}, {"cd", CMD::CD},
			{"pwd", CMD::PWD}, {"info", CMD::INFO}, {"incp", CMD::INCP}, {"outcp", CMD::OUTCP}, 
			{"load", CMD::LOAD}, {"format", CMD::FORMAT},{"xcp", CMD::XCP}, {"exit", CMD::EXIT}
		};
};
