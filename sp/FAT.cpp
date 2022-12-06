#include <algorithm>
#include "FAT.hpp"

void BootSector::init(uint diskSize) {
    mSignature = zero_padded_string("duclong"s, 9);
    mDiskSize = diskSize;
    mClusterSize = CLUSTER_SIZE;
    mClusterCount = (mDiskSize - BootSector::size()) / (mClusterSize * 4);
    mFatEntryCount = mClusterCount;
    mFatStartAddress = BootSector::size();     // offset
    mDataStartAddress = BootSector::size() + mFatEntryCount * sizeof(uint);
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
    // indexed range based for loop
    for (size_t idx = 0; const auto& it : *this) {
        if (it == FAT::FLAG_UNUSED) return idx;
        ++idx;
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

    // construct disk sections
    mBS = std::make_unique<BootSector>();
    mFAT = std::make_unique<FAT>();
    mRootDir = std::make_unique<DirectoryItem>();

    // init disk sections
    this->mBS->init(diskSize);
    this->mFAT->init(mBS->mFatEntryCount);
    this->mRootDir->init("/"s, false, 0, 0);
    mFAT->write_FAT(0, FAT::FLAG_FILE_END);

    // init test file
    DirectoryItem di;
    std::string diContent{"This is content of test.txt. Do what you want with this information though."};
    uint startCluster = mFAT->find_free_index();
    di.init("test.txt", true, diContent.size(), startCluster);
    mFAT->write_FAT(startCluster, FAT::FLAG_FILE_END);
    // increase size by new item
    mRootDir->mSize = di.size();

    if (!mFileStream) {
        std::cout << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

    // write boot sector
    string_to_stream(mFileStream, mBS->mSignature);
    write_to_stream(mFileStream, mBS->mDiskSize);
    write_to_stream(mFileStream, mBS->mClusterSize);
    write_to_stream(mFileStream, mBS->mClusterCount);
    write_to_stream(mFileStream, mBS->mFatEntryCount);
    write_to_stream(mFileStream, mBS->mFatStartAddress);
    write_to_stream(mFileStream, mBS->mDataStartAddress);

    // write FAT
    mFileStream.seekp(mBS->mFatStartAddress);
    for (auto fatEntry : *mFAT) {
        write_to_stream(mFileStream, fatEntry);
    }

    // wipe all clusters
    std::array<char, CLUSTER_SIZE> cluster{};
    cluster.fill('\0');
    mFileStream.seekp(mBS->mDataStartAddress);
    for (size_t i = 0; i < mBS->mClusterCount; ++i) {
        write_to_stream(mFileStream, cluster);
    }

    // write root dir to first cluster
    uint ad1 = mBS->mDataStartAddress;
    mFileStream.seekp(mBS->mDataStartAddress);
    string_to_stream(mFileStream, mRootDir->mFilename);
    write_to_stream(mFileStream, mRootDir->mIsFile);
    write_to_stream(mFileStream, mRootDir->mSize);
    write_to_stream(mFileStream, mRootDir->mStartCluster);

    // write file as file of root dir
    string_to_stream(mFileStream, di.mFilename);
    write_to_stream(mFileStream, di.mIsFile);
    write_to_stream(mFileStream, di.mSize);
    write_to_stream(mFileStream, di.mStartCluster);

    // write file to second cluster
    uint ad2 = mBS->mDataStartAddress + CLUSTER_SIZE * startCluster;
    mFileStream.seekp(mBS->mDataStartAddress + CLUSTER_SIZE * startCluster);
    string_to_stream(mFileStream, diContent);

    mFileStream.flush();
    return true;
}

bool FAT_Filesystem::mount_fs(const std::vector<std::any>& args) {
    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::ate;
    mFileStream.open(mDiskName, mode);

    // construct disk sections
    mBS = std::make_unique<BootSector>();
    mFAT = std::make_unique<FAT>();
    mRootDir = std::make_unique<DirectoryItem>();

    // read info from disk and init
    mBS->init_from_disk(mFileStream, 0);
    mFAT->init(mBS->mFatEntryCount);
    mFAT->init_from_disk(mFileStream, mBS->mFatStartAddress);
    mRootDir->init_from_disk(mFileStream, mBS->mDataStartAddress);

    return true;
}

uint FAT_Filesystem::fs_creat(const std::vector<std::any>& args) {
    auto filename{any_cast<std::string>(args[0])};
    auto isFile{any_cast<bool>(args[1])};
    auto size{any_cast<uint>(args[2])};
    uint startCluster = mFAT->find_free_index();

    uint address = mBS->mDataStartAddress + (startCluster * CLUSTER_SIZE);
    mFAT->write_FAT(startCluster, FAT::FLAG_FILE_END);

    return true;
}

uint FAT_Filesystem::fs_open(const std::vector<std::any>& args) {
    // method takes in name of file,

    if (args.empty()) {
        return 0;
    }

    return 500;
}

uint FAT_Filesystem::fs_read(std::vector<std::any>& args) {
    auto cluster = any_cast<uint>(args[0]);
    auto buffer = any_cast<std::vector<char>>(args[1]);

    if (mFAT->table[cluster] > 0) {
        // recursion

    }

    if (mFAT->table[cluster] == FAT::FLAG_FILE_END) {
        uint ad1 = mBS->mDataStartAddress + (CLUSTER_SIZE * cluster);
//        mFileStream.seekg(mBS->mDataStartAddress + (CLUSTER_SIZE * cluster) + mRootDir->size());
//        read_from_stream(mFileStream, buffer);
        DirectoryItem di;
        di.init_from_disk(mFileStream, ad1);

        DirectoryItem di2;
        di2.mFilename = string_from_stream(mFileStream, 12);
        read_from_stream(mFileStream, di2.mIsFile);
        read_from_stream(mFileStream, di2.mSize);
        read_from_stream(mFileStream, di2.mStartCluster);

        mFileStream.seekp(ad1);
        mFileStream.read(buffer.data(), CLUSTER_SIZE);

        mFileStream.seekp(ad1 + CLUSTER_SIZE);
        mFileStream.read(buffer.data(), CLUSTER_SIZE);

    }

    return buffer.size();
}

uint FAT_Filesystem::fs_write(const std::vector<std::any>& args) {
    auto fd = any_cast<uint>(args[0]);
    auto buffer = any_cast<std::string>(args[1]);
    auto bufferSize = any_cast<uint>(args[2]);


    std::cout << "Writing: ";
    for (const auto& it : args) {
        std::string str{any_cast<std::string>(it)};
        std::cout << str << " ";
        string_to_stream(mFileStream, str);
    }
    std::cout << std::endl;
    mFileStream.flush();

    return false;
}

bool FAT_Filesystem::fs_close(const std::vector<std::any>& args) {

    return false;
}

