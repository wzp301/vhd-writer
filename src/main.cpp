#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
#include <iterator>
#include <sstream>
#include <ctime>
#include <iomanip>

#define ERROR_BASE (9000)
#define OVER_SECTOR_ERROR (ERROR_BASE + 1)
#define OPEN_VHD_ERROR (ERROR_BASE + 2)
#define OPEN_BIN_ERROR (ERROR_BASE + 3)

#define OK_BASE (10000)
#define WRITE_VHD_OK	(OK_BASE + 1)

static const int32_t VHD_FOOTER_BASE = -512;
static const int32_t VHD_COOKIE_OFFSET = (VHD_FOOTER_BASE + 0);
static const int32_t VHD_TIME_OFFSET = (VHD_FOOTER_BASE + 24);
static const int32_t VHD_CREATER_APP_OFFSET = (VHD_FOOTER_BASE + 28);
static const int32_t VHD_CREATER_VER_OFFSET = (VHD_FOOTER_BASE + 32);
static const int32_t VHD_ORGINAL_SIZE_OFFSET = (VHD_FOOTER_BASE + 40);
static const int32_t VHD_CURRENT_SIZE_OFFSET = (VHD_FOOTER_BASE + 48);
static const int32_t VHD_DISK_CYLINDER_OFFSET = (VHD_FOOTER_BASE + 56);
static const int32_t VHD_DISK_HEAD_OFFSET = (VHD_FOOTER_BASE + 58);
static const int32_t VHD_DISK_SECTOR_OFFSET = (VHD_FOOTER_BASE + 59);
static const int32_t VHD_DISK_TYPE_OFFSET = (VHD_FOOTER_BASE + 60);
static const int32_t VHD_CHECK_SUM_OFFSET = (VHD_FOOTER_BASE + 64);
static const int32_t VHD_UUID_OFFSET = (VHD_FOOTER_BASE + 68);

enum class DiskType {
	NONE = 0,
	DEPRECATE,
	FIXED,
	DYNAMIC,
	DIFFERENCING,
	DEPRECATE1,
	DEPRECATE2
};
struct DiskGeometry {
	uint16_t cylinder;
	uint8_t head;
	uint8_t sector;
};

struct CommandLineArg {
	CommandLineArg() {
		mode = 0;
		logic_sector = 0;
		showInfo = false;
	}
	int mode;
	int logic_sector;
	std::string vhd_path;
	std::string bin_path;
	bool showInfo;
};
class Util {
public:
	static uint16_t char2_to_word(const char * arr)
	{
		return arr[0] << 8 | (arr[1] & 0xff);
	}
	static uint32_t char4_to_dword(const char * arr)
	{
		return arr[0] << 24 | (arr[1] << 16 & 0xffffff) | (arr[2] << 8 & 0xffff) | (arr[3] & 0xff);
	}

	static uint64_t char8_to_ddword(const char * arr)
	{
		return arr[0] << 56 | (arr[1] << 48 & 0xffffffffffffff) | (arr[2] << 40 & 0xffffffffffff) | (arr[3] << 32 & 0xffffffffff) | (arr[4] << 24 & 0xffffffff) | (arr[5] << 16 & 0xffffff) | (arr[6] << 8 & 0xffff) | (arr[0] & 0xff);
	}

	static std::string time_to_string(time_t time)
	{
		std::stringstream ss;
		std::tm tm = *std::localtime(&time);
		ss.imbue(std::locale("CHS"));
		ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
		return ss.str();
	}

	static void showUsage()
	{
		std::cerr << "Usage: vhd-writer <option(s)> vhdPath binPath" << std::endl
			<< "Options:\n"
			<< "\t-h,--help \tDESTINATION\tShow this help message" << std::endl
			<< "\t-i,--info \tDESTINATION\tShow information about vhd file" << std::endl
			<< "\t-vp,--vhd_path \tDESTINATION\tSpecify the path of vhd file" << std::endl
			<< "\t-bp,--bin_path \tDESTINATION\tSpecify the path of bin file" << std::endl
			<< "\t-m,--mode \tDESTINATION\tSpecify the writing mode(LBA default)" << std::endl
			<< "\t-s,--sector \tDESTINATION\tSpecify the number of logic sector(Only in LBA mode)" << std::endl;
	}

	static CommandLineArg parseCommandLine(int argc, char ** argv)
	{
		CommandLineArg cla;
		if (argc < 3) {
			Util::showUsage();
			std::exit(-1);
		}
		for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
			if ((arg == "-h") || (arg == "--help")) {
				Util::showUsage();
				std::exit(-1);
			}
			else if ((arg == "-m") || (arg == "--mode")) {
				if (i + 1 < argc) { // Make sure we aren't at the end of argv!
					cla.mode = atoi(argv[++i]); // Increment 'i' so we don't get the argument as the next argv[i].
				}
				else { // Uh-oh, there was no argument to the destination option.
					std::cerr << "--mode option requires one argument." << std::endl;
					std::exit(-1);
				}
			}
			else if ((arg == "-s") || (arg == "--sector")) {
				if (i + 1 < argc) {
					cla.logic_sector = atoi(argv[++i]);
				}
				else {
					std::cerr << "--sector option requires one argument." << std::endl;
					std::exit(-1);
				}

			}
			else if ((arg == "-vp") || (arg == "--vhd_path")) {
				if (i + 1 < argc) {
					cla.vhd_path = argv[++i];
				}
				else {
					std::cerr << "--vhd_path option requires one argument." << std::endl;
					std::exit(-1);
				}

			}
			else if ((arg == "-bp") || (arg == "--bin_path")) {
				if (i + 1 < argc) {
					cla.bin_path = argv[++i];
				}
				else {
					std::cerr << "--bin_path option requires one argument." << std::endl;
					std::exit(-1);
				}

			}
			else if ((arg == "-i") || (arg == "--info")) {
				cla.showInfo = true;
			}
			else {
				;
			}
		}
		return cla;
	}

	static void showCommandLineArg(const CommandLineArg& cla)
	{
		std::cout << "vhd_path = " << cla.vhd_path.c_str() << ", "
			<< "bin_path = " << cla.bin_path.c_str() << ", "
			<< "mode = " << cla.mode << ", "
			<< "logic_sector = " << cla.logic_sector << std::endl;
	}
};

class Vhd {
public:
	Vhd(const std::string& vhdPath)
		: vhd_path_(vhdPath) 
	{
		std::ifstream file(vhd_path_, std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			vhd_file_valid_ = false;
			return;
		}
		vhd_file_valid_ = true;
		// read cookie
		file.seekg(VHD_COOKIE_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(cookie_), sizeof(cookie_));

		// read time
		file.seekg(VHD_TIME_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&time_), sizeof(time_));
		time_ = Util::char4_to_dword(reinterpret_cast<char*>(&time_));

		// read application of creator
		file.seekg(VHD_CREATER_APP_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&creater_app_), sizeof(creater_app_));

		// read version of creator
		file.seekg(VHD_CREATER_VER_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&creater_version_), sizeof(creater_version_));
		creater_version_ = Util::char4_to_dword(reinterpret_cast<char*>(&creater_version_));

		// read orginal size
		file.seekg(VHD_ORGINAL_SIZE_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&orginal_size_), sizeof(orginal_size_));
		orginal_size_ = Util::char8_to_ddword(reinterpret_cast<char*>(&orginal_size_));

		// read current size
		file.seekg(VHD_CURRENT_SIZE_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&current_size_), sizeof(current_size_));
		current_size_ = Util::char8_to_ddword(reinterpret_cast<char*>(&current_size_));

		// read disk geometry
		file.seekg(VHD_DISK_CYLINDER_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&disk_geometry_.cylinder), sizeof(disk_geometry_.cylinder));
		disk_geometry_.cylinder = Util::char2_to_word(reinterpret_cast<char*>(&disk_geometry_.cylinder));

		file.seekg(VHD_DISK_HEAD_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&disk_geometry_.head), sizeof(disk_geometry_.head));
		
		file.seekg(VHD_DISK_SECTOR_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&disk_geometry_.sector), sizeof(disk_geometry_.sector));

		logic_sector_ = (disk_geometry_.head * disk_geometry_.cylinder * disk_geometry_.sector) - 1;

		// read type of disk
		file.seekg(VHD_DISK_TYPE_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&disk_type_), sizeof(disk_type_));
		disk_type_ = Util::char4_to_dword(reinterpret_cast<char*>(&disk_type_));

		// read check sum
		file.seekg(VHD_CHECK_SUM_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&check_sum_), sizeof(check_sum_));
		check_sum_ = Util::char4_to_dword(reinterpret_cast<char*>(&check_sum_));

		// read uuid
		file.seekg(VHD_UUID_OFFSET, std::ios::end);
		file.read(reinterpret_cast<char*>(&uuid), sizeof(uuid));

		file.close();
	}
	
	void info(std::string& info) 
	{
		std::stringstream ss;
		if (vhd_file_valid_) {
			ss << "\tcookies : " << cookie_ << std::endl
				//<< "\ttime : " << Util::time_to_string(time_).c_str() << std::endl
				<< "\tcreater app : " << creater_app_ << std::endl
				<< "\tcylinder : " << disk_geometry_.cylinder << std::endl
				<< "\thead : " << static_cast<int>(disk_geometry_.head) << std::endl
				<< "\tsector : " << static_cast<int>(disk_geometry_.sector) << std::endl
				<< "\tlogic sector : " << logic_sector_ << std::endl
				<< "\ttype : " << disk_type_ << " (2->fixed, 3->dynamic)" << std::endl;
		}
		else {
			ss << "Opening vhd file error! vhdPath is" << vhd_path_.c_str() << std::endl;
		}
		info = ss.str();
	}

	DiskGeometry get_disk_geometry() const {
		return disk_geometry_;
	}
	const std::string& get_vhd_path() const {
		return vhd_path_;
	}
	int writeByLBA(const std::vector<uint8_t> data, uint32_t sectorIndex)
	{
		if (sectorIndex >= logic_sector_) {
			return OVER_SECTOR_ERROR;
		}
		std::ofstream file(vhd_path_, std::ios::out | std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			return OPEN_VHD_ERROR;
		}
		file.seekp(512 * sectorIndex);
		file.write((char *)data.data(), data.size());
		file.close();
		return WRITE_VHD_OK;
	}
private:
	std::string vhd_path_;
	bool vhd_file_valid_;
	char cookie_[9];
	uint32_t time_;
	char creater_app_[4];
	uint32_t creater_version_;
	uint64_t orginal_size_;
	uint64_t current_size_;
	uint32_t disk_type_;
	uint16_t cylinder_;
	DiskGeometry disk_geometry_;
	uint32_t check_sum_;
	uint8_t uuid[16];
	uint32_t logic_sector_;
};

class VhdWriter {
public:
	static void writeWithBinPath(Vhd& vhd, const std::string& binPath, uint32_t sectorIndex, int mode)
	{
		std::ifstream binFile(binPath);
		if (!binFile.is_open()) {
			std::cout << "Opening bin file error! binPath is " << binPath.c_str() << std::endl;
			std::exit(-1);
		}
		std::vector<uint8_t> data;
		std::istream_iterator<uint8_t> start(binFile), end;
		std::copy(start, end, std::back_inserter(data));
		binFile.close();

		int ret = WRITE_VHD_OK;
		if (mode == 1) {
			// CHS mode
			// TODO
			std::cout << "Wait for the implementation!" << std::endl;
			std::exit(-1);
		}
		else {
			// LBA mode
			ret = vhd.writeByLBA(data, sectorIndex);
		}
		if (ret == OVER_SECTOR_ERROR) {
			std::cout << "error: OVER_SECTOR_ERROR! The sector input is " << sectorIndex << ", but vhd sector is " << vhd.get_disk_geometry().sector << std::endl;
		}
		else if (ret == OPEN_VHD_ERROR) {
			std::cout << "error: Opening vhd file error! vhdPath is" <<  vhd.get_vhd_path().c_str() << std::endl;
		}
		else if (ret == WRITE_VHD_OK) {
			std::cout << "info : Writing " << data.size() << " bytes to sector " << sectorIndex << " of vhd." << std::endl;
		}
		else {
			std::cout << "error : Unknown error occurred!" << std::endl;
		}
	}
};

int main(int argc, char ** argv)
{
	CommandLineArg cla = Util::parseCommandLine(argc, argv);

	Vhd vhd(cla.vhd_path);
	
	if (cla.showInfo) {	// Only show information about vhd file
		std::string info;
		vhd.info(info);
		std::cout << info.c_str();
	}
	else {
		VhdWriter::writeWithBinPath(vhd, cla.bin_path, cla.logic_sector, cla.mode);
	}
	
	return 0;
}