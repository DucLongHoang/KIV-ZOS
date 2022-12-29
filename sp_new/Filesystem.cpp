#include <ranges>
#include "Filesystem.hpp"

// Start : BootSector
void BootSector::init(uint diskSize) {
    mSignature = Utils::zero_padded_string("duclong", SIGNATURE_LEN);
    mDiskSize = diskSize;
    mClusterSize = CLUSTER_SIZE;
    mClusterCount = (mDiskSize - BootSector::size()) / (mClusterSize * 4);
    mFatStartAddress = BootSector::size();
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

void BootSector::write_to_disk(std::fstream &stream) {
    Utils::string_to_stream(stream, mSignature);
    Utils::write_to_stream(stream, mDiskSize, mClusterSize,mClusterCount,
                           mFatStartAddress, mDataStartAddress);
}
// End : BootSector

// Start : FAT
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

void FAT::write_to_disk(std::fstream &stream) {
    for (auto fatEntry : *this) {
        Utils::write_to_stream(stream, fatEntry);
    }
}
// End : FAT

// Start : DirEntry
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

void DirEntry::write_to_disk(std::fstream &stream) {
    Utils::string_to_stream(stream, mFilename);
    Utils::write_to_stream(stream, mIsFile, mSize, mStartCluster);
}
// End : DirEntry

void Filesystem::wipe_clusters() {
    // create empty cluster
    std::array<char, CLUSTER_SIZE> emptyCluster{};
    emptyCluster.fill('\0');
    // move seek to correct position
    mFileStream.seekp(mBS.mDataStartAddress);
    auto clusterView = std::ranges::iota_view{0u, mBS.mClusterCount};
    // repeat wiping cluster
    std::for_each(clusterView.begin(), clusterView.end(), [this, &emptyCluster](auto i) {
        Utils::write_to_stream(mFileStream, emptyCluster);
    });
}

void Filesystem::init(uint size) {
    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc;
    mFileStream.open(mDiskName, mode);

    // construct disk sections
    mBS = BootSector();
    mFAT = FAT();
    mRootDir = DirEntry();

    // init disk sections
    mBS.init(size);
    mFAT.init(mBS.mClusterCount);
    mRootDir.init("/", false, 0, 0);
    mFAT.write_FAT(0, FAT::FLAG_FILE_END);

    if (!mFileStream) {
        std::cout << "Error opening file" << std::endl;
        exit(EXIT_FAILURE);
    }

    mBS.write_to_disk(mFileStream);

    mFileStream.seekp(mBS.mFatStartAddress);
    mFAT.write_to_disk(mFileStream);

    wipe_clusters();
    mFileStream.seekp(mBS.mDataStartAddress);
    mRootDir.write_to_disk(mFileStream);
//    init_test_files();

    mFileStream.flush();
}

void Filesystem::mount() {
    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::ate;
    mFileStream.open(mDiskName, mode);

    // construct disk sections
    mBS = BootSector();
    mFAT = FAT();
    mRootDir = DirEntry();

    // read info from disk and init
    mBS.mount(mFileStream, 0);
    mFAT.init(mBS.mClusterCount);
    mFAT.mount(mFileStream, mBS.mFatStartAddress);
    mRootDir.mount(mFileStream, mBS.mDataStartAddress);
}

void Filesystem::create_dir_entry(const std::string& name, bool isFile, const std::string& content) {
    DirEntry dirEntry;
    uint startCluster = mFAT.find_free_index();
    dirEntry.init(name, isFile, content.size(), startCluster);
}

void Filesystem::init_default_files() {
    // START: file 01
    DirEntry dirEntry1;
    std::string diContent{"This is content of test.txt. Do what you want with this information."};
    uint startCluster = mFAT.find_free_index();
    dirEntry1.init("test.txt", true, diContent.size(), startCluster);

    mFAT.write_FAT(startCluster, FAT::FLAG_FILE_END);
    mRootDir.mSize += dirEntry1.size();
//
//    // write file to cluster
//
//    // START: file 02
//    DirectoryItem di2;
//    startCluster = mFAT.find_free_index();
//    di2.init("home", false, 0, startCluster);
//
//    mFAT.write_FAT(startCluster, FAT::FLAG_FILE_END);
//    mRootDir.mSize += di2.size();
//
//    // START: file 03
//    DirectoryItem di3;
//    std::string diContent3{"As expected, the random sampling method has the worst result, with several points overlapping and being too close to each other. The Poisson disk sampling does not have a problem with overlapping points but due to its random nature, the polygon is populated non-uniformly. The k-means method yields the best results with all points being distributed evenly across the whole polygon."};
//    startCluster = mFAT.find_free_index();
//    di3.init("thesis.txt", true, diContent3.size(), startCluster);
//
//    mFAT.write_FAT(startCluster, FAT::FLAG_FILE_END);
//    di2.mSize += di3.size();
//
//    // writing to disk
//    write_root_dir();
//
//    write_directory_item(dirEntry1);
//    write_directory_item(di2);
//
//    mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE * dirEntry1.mStartCluster);
//    string_to_stream(mFileStream, diContent);
//
//    mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE * di2.mStartCluster);
//    write_directory_item(di3);
//
//    mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE * di3.mStartCluster);
//    string_to_stream(mFileStream, diContent3);
}