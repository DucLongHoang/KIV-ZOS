#include <ranges>
#include <algorithm>
#include "FAT.hpp"

void BootSector::init(uint diskSize) {
    mSignature = zero_padded_string("duclong"s, SIGNATURE_LEN);
    mDiskSize = diskSize;
    mClusterSize = CLUSTER_SIZE;
    mClusterCount = (mDiskSize - BootSector::size()) / (mClusterSize * 4);
    mFatEntryCount = mClusterCount;
    mFatStartAddress = BootSector::size();     // offset
    mDataStartAddress = BootSector::size() + mFatEntryCount * sizeof(uint);
}

void BootSector::init_from_disk(std::fstream& stream, uint pos) {
    stream.seekg(pos);
    mSignature = string_from_stream(stream, SIGNATURE_LEN);
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

uint FAT::find_free_index() const {
    // indexed range based for loop
    for (size_t idx = 0; const auto& it : *this) {
        if (it == FAT::FLAG_UNUSED) return idx;
        ++idx;
    }
    return FAT::FLAG_NO_FREE_SPACE;
}

void DirectoryItem::init(const std::string& filename, bool isFile, uint size, uint startCLuster) {
    mFilename = zero_padded_string(filename, FILENAME_LEN);
    mIsFile = isFile;
    mSize = size;
    mStartCluster = startCLuster;
}

void DirectoryItem::init_from_disk(std::fstream& stream, uint pos) {
    stream.seekg(pos);
    mFilename = string_from_stream(stream, FILENAME_LEN);
    read_from_stream(stream, mIsFile);
    read_from_stream(stream, mSize);
    read_from_stream(stream, mStartCluster);
}

void FAT_Filesystem::wipe_clusters() {
    // wipe all clusters
    std::array<char, CLUSTER_SIZE> cluster{};
    cluster.fill('\0');

    mFileStream.seekp(mBS->mDataStartAddress);
    for ([[maybe_unused]] const auto& i : std::ranges::iota_view{0u, mBS->mClusterCount}) {
        write_to_stream(mFileStream, cluster);
    }
}

void FAT_Filesystem::init_test_files() {
    // START: file 01
    DirectoryItem di;
    std::string diContent{"This is content of test.txt. Do what you want with this information."};
    uint startCluster = mFAT->find_free_index();
    di.init("test.txt", true, diContent.size(), startCluster);

    mFAT->write_FAT(startCluster, FAT::FLAG_FILE_END);
    mRootDir->mSize += di.size();

    // write file to cluster

    // START: file 02
    DirectoryItem di2;
    startCluster = mFAT->find_free_index();
    di2.init("home", false, 0, startCluster);

    mFAT->write_FAT(startCluster, FAT::FLAG_FILE_END);
    mRootDir->mSize += di2.size();

    // START: file 03
    DirectoryItem di3;
    std::string diContent3{"As expected, the random sampling method has the worst result, with several points overlapping and being too close to each other. The Poisson disk sampling does not have a problem with overlapping points but due to its random nature, the polygon is populated non-uniformly. The k-means method yields the best results with all points being distributed evenly across the whole polygon."};
    startCluster = mFAT->find_free_index();
    di3.init("thesis.txt", true, diContent3.size(), startCluster);

    mFAT->write_FAT(startCluster, FAT::FLAG_FILE_END);
    di2.mSize += di3.size();

    // writing to disk
    write_root_dir();

    write_directory_item(di);
    write_directory_item(di2);

    mFileStream.seekp(mBS->mDataStartAddress + CLUSTER_SIZE * di.mStartCluster);
    string_to_stream(mFileStream, diContent);

    mFileStream.seekp(mBS->mDataStartAddress + CLUSTER_SIZE * di2.mStartCluster);
    write_directory_item(di3);

    mFileStream.seekp(mBS->mDataStartAddress + CLUSTER_SIZE * di3.mStartCluster);
    string_to_stream(mFileStream, diContent3);
}

void FAT_Filesystem::write_boot_sector() {
    string_to_stream(mFileStream, mBS->mSignature);
    write_to_stream(mFileStream, mBS->mDiskSize);
    write_to_stream(mFileStream, mBS->mClusterSize);
    write_to_stream(mFileStream, mBS->mClusterCount);
    write_to_stream(mFileStream, mBS->mFatEntryCount);
    write_to_stream(mFileStream, mBS->mFatStartAddress);
    write_to_stream(mFileStream, mBS->mDataStartAddress);
}

void FAT_Filesystem::write_FAT() {
    for (auto fatEntry : *mFAT) {
        write_to_stream(mFileStream, fatEntry);
    }
}

void FAT_Filesystem::write_root_dir() {
    string_to_stream(mFileStream, mRootDir->mFilename);
    write_to_stream(mFileStream, mRootDir->mIsFile);
    write_to_stream(mFileStream, mRootDir->mSize);
    write_to_stream(mFileStream, mRootDir->mStartCluster);
}

void FAT_Filesystem::write_directory_item(DirectoryItem& dirItem) {
    string_to_stream(mFileStream, dirItem.mFilename);
    write_to_stream(mFileStream, dirItem.mIsFile);
    write_to_stream(mFileStream, dirItem.mSize);
    write_to_stream(mFileStream, dirItem.mStartCluster);
}

bool FAT_Filesystem::init_fs(const std::vector<std::any>& args) {
    uint multiplier = (any_cast<std::string>(args.back()) == "MB"s) ? 1_MB : 1_KB;
    uint diskSize = std::stoi(any_cast<std::string>(args.front())) * multiplier;

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

    if (!mFileStream) {
        std::cout << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

    write_boot_sector();
    mFileStream.seekp(mBS->mFatStartAddress);
    write_FAT();
    wipe_clusters();
    mFileStream.seekp(mBS->mDataStartAddress);
//    write_root_dir();

    init_test_files();

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
    auto& buffer = any_cast<std::vector<char>&>(args[1]);
    uint size = buffer.size();

    if (mFAT->table[cluster] > 0) {
        // recursion

    }

    if (mFAT->table[cluster] == FAT::FLAG_FILE_END) {
        uint address = mBS->mDataStartAddress + (CLUSTER_SIZE * cluster);
        mFileStream.seekp(address);
        mFileStream.read(buffer.data(), size);
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