#include <algorithm>
#include "FAT.hpp"

void BootSector::init(uint diskSize) {
    mSignature = zero_padded_string("duclong"s, 9);
    mDiskSize = diskSize;
    mClusterSize = CLUSTER_SIZE;
    mClusterCount = (mDiskSize - BootSector::SIZE) / (mClusterSize * 4);
    mFatEntryCount = mClusterCount;
    mFatStartAddress = BootSector::SIZE;     // offset
    mDataStartAddress = BootSector::SIZE + mFatEntryCount * sizeof(uint);
}

void BootSector::init_from_disk(std::fstream& stream, uint pos) {
    stream.seekg(pos);
    mSignature = string_from_stream(stream, 9);
    read_from_stream(stream, mDiskSize);
    read_from_stream(stream, mClusterSize);
    read_from_stream(stream, mClusterCount);
    read_from_stream(stream, mFatEntryCount);
    read_from_stream(stream, mFatStartAddress);
    read_from_stream(stream, mDataStartAddress);
}

void FAT::init(uint fatEntryCount) {
    table.resize(fatEntryCount, FAT::FLAG_UNUSED);
    SIZE = table.size() * sizeof(uint);
}

void FAT::init_from_disk(std::fstream& stream, uint pos) {
    stream.seekg(pos);
    for (int& fatEntry : table) {
        read_from_stream(stream, fatEntry);
    }
}

void FAT::write_FAT(uint idx, int idxOrFlag) {
    table[idx] = idxOrFlag;
}

uint FAT::find_free_index() {
    for (size_t i = 0; i < table.size(); ++i) {
        if (table[i] == FAT::FLAG_UNUSED) return i;
    }
    return FAT::FLAG_NO_FREE_SPACE;
}

void DirectoryItem::init(const std::string& filename, bool isFile, uint size, uint startCLuster) {
    mFilename = zero_padded_string(filename, 12);
    mIsFile = isFile;
    mSize = size;
    mStartCluster = startCLuster;
}

void DirectoryItem::init_from_disk(std::fstream& stream, uint pos) {
    stream.seekg(pos);
    mFilename = string_from_stream(stream, 12);
    read_from_stream(stream, mIsFile);
    read_from_stream(stream, mSize);
    read_from_stream(stream, mStartCluster);
}

bool FAT_Filesystem::init_fs(const std::vector<std::any>& args) {
    auto multiplier = (any_cast<std::string>(args.back()) == "MB"s) ? 1_MB : 1_KB;
    auto diskSize = std::stoi(any_cast<std::string>(args.front())) * multiplier;

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

    DirectoryItem di;
    std::string diContent{"This is content of test.txt. Do what you want with this information though."};
    di.init("test.txt", true, diContent.size(), 1);
    mFAT.write_FAT(1, FAT::FLAG_FILE_END);

    // write file as file of root dir
//    mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE);
    string_to_stream(mFileStream, di.mFilename);
    write_to_stream(mFileStream, di.mIsFile);
    write_to_stream(mFileStream, di.mSize);
    write_to_stream(mFileStream, di.mStartCluster);

    // write file to second cluster
    mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE);
    string_to_stream(mFileStream, diContent);


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

uint FAT_Filesystem::fs_creat(const std::vector<std::any>& args) {
    auto filename{any_cast<std::string>(args[0])};
    auto isFile{any_cast<bool>(args[1])};
    uint size;
    uint startCluster = mFAT.find_free_index();

    uint address = mBS.mDataStartAddress + (startCluster * CLUSTER_SIZE);
    mFAT.write_FAT(startCluster, FAT::FLAG_FILE_END);

    return false;
}

uint FAT_Filesystem::fs_open(const std::vector<std::any>& args) {

    return false;
}

uint FAT_Filesystem::fs_read(const std::vector<std::any>& args) {

    return false;
}

uint FAT_Filesystem::fs_write(const std::vector<std::any>& args) {
    auto fd = any_cast<uint>(args[0]);
    auto buffer = any_cast<std::string>(args[1]);
    auto bufferSize = any_cast<uint>(args[2]);


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

