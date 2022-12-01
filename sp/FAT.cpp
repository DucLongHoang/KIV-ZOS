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

void FAT::init(unsigned int fatEntryCount) {
    table.resize(fatEntryCount, FAT::FLAG_UNUSED);
}

void DirectoryItem::init(const std::string& filename, bool isFile, int size, int startCLuster) {
    mFilename = zero_padded_string(filename, 12);
    mIsFile = isFile;
    mSize = size;
    mStartCluster = startCLuster;
}

bool FAT_Filesystem::init_fs(const std::vector<std::string>& args) {
    unsigned int multiplier = (args.back() == "MB"s) ? 1_MB : 1_KB;
    unsigned int diskSize = std::stoi(args.front()) * multiplier;

    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc;
    mFileStream.open(mDiskName, mode);

    this->mBS.init(diskSize);
    this->mFAT.init(mBS.mFatEntryCount);
    this->mRootDir.init("/"s, false, 0, 0);

    if (!mFileStream) {
        std::cout << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

    write_to_stream(mFileStream, mBS.mSignature);
    write_to_stream(mFileStream, mBS.mDiskSize);
    write_to_stream(mFileStream, mBS.mClusterSize);
    write_to_stream(mFileStream, mBS.mClusterCount);
    write_to_stream(mFileStream, mBS.mFatEntryCount);
    write_to_stream(mFileStream, mBS.mFatStartAddress);
    write_to_stream(mFileStream, mBS.mDataStartAddress);

    mFileStream.seekp(mBS.mFatStartAddress);
    for_each(mFAT.table.begin(), mFAT.table.end(), [this](int fatEntry) {
        write_to_stream(mFileStream, fatEntry);
    });

    mFileStream.flush();
    return true;
}

bool FAT_Filesystem::mount_fs(const std::vector<std::string>& args) {
    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::ate;
    mFileStream.open(mDiskName, mode);

    // read info from disk and init
    // init BS

    // init FAT
    // init root dir

    return true;
}

bool FAT_Filesystem::fs_creat(const std::vector<std::string>& args) {

    return false;
}

bool FAT_Filesystem::fs_open(const std::vector<std::string>& args) {

    return false;
}

bool FAT_Filesystem::fs_read(const std::vector<std::string>& args) {

    return false;
}

bool FAT_Filesystem::fs_write(const std::vector<std::string>& args) {
    std::cout << "Writing: ";
    std::for_each(args.begin(), args.end(), [this, args](const std::string& s) {
        std::cout << s << " ";
        write_to_stream(mFileStream, s, s.length());
    });
    std::cout << std::endl;
    mFileStream.flush();

    return false;
}

bool FAT_Filesystem::fs_close(const std::vector<std::string>& args) {

    return false;
}

