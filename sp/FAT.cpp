#include <algorithm>
#include "FAT.hpp"

void BootSector::init(unsigned int diskSize) {
    mSignature = zero_padded_string("duclong"s, 9);
    mDiskSize = diskSize;
    mClusterSize = CLUSTER_SIZE;
    mClusterCount = (mDiskSize - BootSector::SIZE) / (mClusterSize * 4);
    mFatEntryCount = mClusterCount;
    mFatStartAddress = BootSector::SIZE;     // offset
    mDataStartAddress = BootSector::SIZE + mFatEntryCount * sizeof(unsigned int);
}

void BootSector::init_from_disk(std::fstream& stream, unsigned int pos) {
    stream.seekg(pos);
    mSignature = string_from_stream(stream, 9);
    read_from_stream(stream, mDiskSize);
    read_from_stream(stream, mClusterSize);
    read_from_stream(stream, mClusterCount);
    read_from_stream(stream, mFatEntryCount);
    read_from_stream(stream, mFatStartAddress);
    read_from_stream(stream, mDataStartAddress);
}

void FAT::init(unsigned int fatEntryCount) {
    table.resize(fatEntryCount, FAT::FLAG_UNUSED);
    SIZE = table.size() * sizeof(unsigned int);
}

void FAT::init_from_disk(std::fstream& stream, unsigned int pos) {
    stream.seekg(pos);
    for (int& fatEntry : table) {
        read_from_stream(stream, fatEntry);
    }
}

void FAT::write_FAT(unsigned int idx, int idxOrFlag) {
    table[idx] = idxOrFlag;
}

unsigned int FAT::find_free_index() {
    for (size_t i = 0; i < table.size(); ++i) {
        if (table[i] == FAT::FLAG_UNUSED) return i;
    }
    return FAT::FLAG_NO_FREE_SPACE;
}

void DirectoryItem::init(const std::string& filename, bool isFile, int size, int startCLuster) {
    mFilename = zero_padded_string(filename, 12);
    mIsFile = isFile;
    mSize = size;
    mStartCluster = startCLuster;
}

void DirectoryItem::init_from_disk(std::fstream& stream, unsigned int pos) {
    stream.seekg(pos);
    mFilename = string_from_stream(stream, 12);
    read_from_stream(stream, mIsFile);
    read_from_stream(stream, mSize);
    read_from_stream(stream, mStartCluster);
}

bool FAT_Filesystem::init_fs(const std::vector<std::any>& args) {


    unsigned int multiplier = (any_cast<std::string>(args.back()) == "MB"s) ? 1_MB : 1_KB;
    unsigned int diskSize = std::stoi(any_cast<std::string>(args.front())) * multiplier;

    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc;
    mFileStream.open(mDiskName, mode);

    this->mBS.init(diskSize);
    this->mFAT.init(mBS.mFatEntryCount);
    this->mRootDir.init("/"s, false, 0, 0);
    mFAT.write_FAT(0, FAT::FLAG_FILE_END);

    if (!mFileStream) {
        std::cout << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

    // write boot sector
    string_to_stream(mFileStream, mBS.mSignature);
    write_to_stream(mFileStream, mBS.mDiskSize);
    write_to_stream(mFileStream, mBS.mClusterSize);
    write_to_stream(mFileStream, mBS.mClusterCount);
    write_to_stream(mFileStream, mBS.mFatEntryCount);
    write_to_stream(mFileStream, mBS.mFatStartAddress);
    write_to_stream(mFileStream, mBS.mDataStartAddress);

    // write FAT
    mFileStream.seekp(mBS.mFatStartAddress);
    for_each(mFAT.table.begin(), mFAT.table.end(), [this](int fatEntry) {
        write_to_stream(mFileStream, fatEntry);
    });

    // wipe all clusters
    std::array<char, CLUSTER_SIZE> cluster{};
    cluster.fill('\0');
    mFileStream.seekp(mBS.mDataStartAddress);
    for (size_t i = 0; i < mBS.mClusterCount; ++i) {
        write_to_stream(mFileStream, cluster);
    }

    // write root dir to first cluster
    mFileStream.seekp(mBS.mDataStartAddress);
    string_to_stream(mFileStream, mRootDir.mFilename);
    write_to_stream(mFileStream, mRootDir.mIsFile);
    write_to_stream(mFileStream, mRootDir.mSize);
    write_to_stream(mFileStream, mRootDir.mStartCluster);

    mFileStream.flush();
    return true;
}

bool FAT_Filesystem::mount_fs(const std::vector<std::any>& args) {
    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::ate;
    mFileStream.open(mDiskName, mode);

    // read info from disk and init
    mBS.init_from_disk(mFileStream, 0);
    mFAT.init(mBS.mFatEntryCount);
    mFAT.init_from_disk(mFileStream, mBS.mFatStartAddress);
    mRootDir.init_from_disk(mFileStream, mBS.mDataStartAddress);

    return true;
}

unsigned int FAT_Filesystem::fs_creat(const std::vector<std::any>& args) {
    std::string filename{any_cast<std::string>(args[0])};
    bool isFile{any_cast<bool>(args[1])};
    unsigned int size;
    unsigned int startCluster = mFAT.find_free_index();


    return false;
}

unsigned int FAT_Filesystem::fs_open(const std::vector<std::any>& args) {

    return false;
}

unsigned int FAT_Filesystem::fs_read(const std::vector<std::any>& args) {

    return false;
}

unsigned int FAT_Filesystem::fs_write(const std::vector<std::any>& args) {



    std::cout << "Writing: ";

    std::for_each(args.begin(), args.end(), [this, args](const std::any& a) {
        std::string str{any_cast<std::string>(a)};
        std::cout << str << " ";
        string_to_stream(mFileStream, str);
    });

    std::cout << std::endl;
    mFileStream.flush();

    return false;
}

bool FAT_Filesystem::fs_close(const std::vector<std::any>& args) {

    return false;
}

