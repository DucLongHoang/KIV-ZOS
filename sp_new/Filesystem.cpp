#include <ranges>
#include "Filesystem.hpp"

void BootSector::init(uint diskSize) {
    mSignature = Utils::zero_padded_string("duclong", SIGNATURE_LEN);
    mDiskSize = diskSize;
    mClusterSize = CLUSTER_SIZE;
    mClusterCount = (mDiskSize - BootSector::size()) / (mClusterSize * 4);
    mFatStartAddress = BootSector::size();     // offset
    mDataStartAddress = BootSector::size() + mClusterCount * sizeof(uint);
}

void BootSector::mount(std::fstream& stream, uint pos) {
    stream.seekg(pos);
    mSignature = Utils::string_from_stream(stream, SIGNATURE_LEN);
    mDiskSize = Utils::read_from_stream<uint>(stream);
    mClusterSize = Utils::read_from_stream<uint>(stream);
    mClusterCount = Utils::read_from_stream<uint>(stream);
    mFatStartAddress = Utils::read_from_stream<uint>(stream);
    mDataStartAddress = Utils::read_from_stream<uint>(stream);
}

void FAT::init(uint fatEntryCount) {
    table.resize(fatEntryCount, FAT::FLAG_UNUSED);
}

void FAT::mount(std::fstream& stream, uint pos) {
    stream.seekg(pos);
    for (int& fatEntry : table) {
        fatEntry = Utils::read_from_stream<int>(stream);
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

void DirEntry::init(const std::string& filename, bool isFile, uint size, uint startCLuster) {
    mFilename = Utils::zero_padded_string(filename, FILENAME_LEN);
    mIsFile = isFile;
    mSize = size;
    mStartCluster = startCLuster;
}

void DirEntry::mount(std::fstream &stream, uint pos) {
    stream.seekg(pos);
    mFilename = Utils::string_from_stream(stream, FILENAME_LEN);
    mIsFile = Utils::read_from_stream<bool>(stream);
    mSize = Utils::read_from_stream<uint>(stream);
    mStartCluster = Utils::read_from_stream<uint>(stream);
}

void Filesystem::wipe_clusters() {
    // create empty cluster
    std::array<char, CLUSTER_SIZE> emptyCluster{};
    emptyCluster.fill('\0');
    // move seek to correct position
    mFileStream.seekp(mBS->mDataStartAddress);
    auto clusterView = std::ranges::iota_view{0u, mBS->mClusterCount};
    // repeat wiping cluster
    std::for_each(clusterView.begin(), clusterView.end(), [this, &emptyCluster](auto i) {
        Utils::write_to_stream(mFileStream, emptyCluster);
    });
}

void Filesystem::init(uint size) {
    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc;
    mFileStream.open(mDiskName, mode);

    // construct disk sections
    mBS = std::make_unique<BootSector>();
    mFAT = std::make_unique<FAT>();
    mRootDir = std::make_unique<DirEntry>();

    // init disk sections
    this->mBS->init(size);
    this->mFAT->init(mBS->mClusterCount);
    this->mRootDir->init("/", false, 0, 0);
    mFAT->write_FAT(0, FAT::FLAG_FILE_END);

    if (!mFileStream) {
        std::cout << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

//    write_boot_sector();
    mFileStream.seekp(mBS->mFatStartAddress);
//    write_FAT();
    wipe_clusters();
    mFileStream.seekp(mBS->mDataStartAddress);
//    write_root_dir();

//    init_test_files();

    mFileStream.flush();
}

void Filesystem::mount() {
    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::ate;
    mFileStream.open(mDiskName, mode);

    // construct disk sections
    mBS = std::make_unique<BootSector>();
    mFAT = std::make_unique<FAT>();
    mRootDir = std::make_unique<DirEntry>();

    // read info from disk and init
    mBS->mount(mFileStream, 0);
    mFAT->init(mBS->mClusterCount);
    mFAT->mount(mFileStream, mBS->mFatStartAddress);
    mRootDir->mount(mFileStream, mBS->mDataStartAddress);
}