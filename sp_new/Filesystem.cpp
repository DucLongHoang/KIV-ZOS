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

void FAT::write_FAT(uint idx, uint fileSize) {
    if (fileSize < CLUSTER_SIZE)
        table[idx] = FAT::FLAG_FILE_END;
    else {
        uint nextFreeCluster = find_free_index();
        do {
            table[idx] = nextFreeCluster;
            idx = nextFreeCluster;
            nextFreeCluster = find_free_index();
            fileSize -= CLUSTER_SIZE;
        }
        while (fileSize > 0);
    }
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

uint FAT::find_index_of(const std::string &str, uint startingPoint) const {
    return  0;
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

void DirEntry::write_content_to_disk(std::fstream &stream, uint dataStartAddress, const std::vector<uint>& clusters, const std::string& content) {
    if (content.size() <= CLUSTER_SIZE) {
        stream.seekp(dataStartAddress + CLUSTER_SIZE * this->mStartCluster);
        Utils::string_to_stream(stream, content);
    }
    else {
        // splitting file content into cluster sized bites
        std::string part = content.substr(0, CLUSTER_SIZE);
        std::string rest = content.substr(CLUSTER_SIZE + 1);
        for (auto cluster: clusters) {
            stream.seekp(dataStartAddress + CLUSTER_SIZE * cluster);
            Utils::string_to_stream(stream, part);

            if (rest.size() > CLUSTER_SIZE) {
                part = rest.substr(0, CLUSTER_SIZE);
                rest = rest.substr(CLUSTER_SIZE + 1);
            } else {
                part = rest;
            }
        }
    }
}

DirEntry::operator bool() const {
    return mFilename != Utils::zero_padded_string("", FILENAME_LEN);
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
    mFAT.write_FAT(0, 0);

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

    init_default_files();
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

DirEntry Filesystem::get_dir_entry(uint cluster) {
    DirEntry dirEntry;
    dirEntry.mount(mFileStream, mBS.mDataStartAddress + cluster * CLUSTER_SIZE);
    return dirEntry;
}

void Filesystem::create_dir_entry(uint parentCluster, const std::string& name, bool isFile, const std::string& content) {
    // create new file
    DirEntry newDirEntry;
    uint startCluster = mFAT.find_free_index();
    newDirEntry.init(name, isFile, content.size(), startCluster);

    // increase size of parent dir by DirEntry size
    DirEntry parentDirEntry = get_dir_entry(parentCluster);
    parentDirEntry.mSize += newDirEntry.size();

    // set content of new file into FAT table
    mFAT.write_FAT(startCluster, content.size());
    mFileStream.seekp(mBS.mFatStartAddress);
    mFAT.write_to_disk(mFileStream);

    // save new info of parent dir to disk
    mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE * parentDirEntry.mStartCluster);
    parentDirEntry.write_to_disk(mFileStream);

    // write new file meta-info as content of parent dir
    mFileStream.seekp(parentDirEntry.mSize - parentDirEntry.size(), std::ios::cur);     // some magic
    newDirEntry.write_to_disk(mFileStream);

    // save new file content into disk
    auto clusters = get_cluster_locations(newDirEntry.mFilename);
    newDirEntry.write_content_to_disk(mFileStream, mBS.mDataStartAddress, clusters, content);
}

std::vector<uint> Filesystem::get_cluster_locations(const std::string& dirEntryName) {
    std::vector<uint> clusters{};
    return clusters;
}

void Filesystem::init_default_files() {
    // START: file 01
    DirEntry de1;
    std::string de1Content{"This is content of test.txt. Do what you want with this information."};
    uint startCluster = mFAT.find_free_index();
    de1.init("test.txt", true, de1Content.size(), startCluster);
    mFAT.write_FAT(startCluster, de1Content.size());
    mRootDir.mSize += de1.size();

    // START: file 02
    DirEntry de2;
    startCluster = mFAT.find_free_index();
    de2.init("home", false, 0, startCluster);
    mFAT.write_FAT(startCluster, 0);
    mRootDir.mSize += de2.size();

    // START: file 03
    DirEntry de3;
    std::string de3Content{"As expected, the random sampling method has the worst result, with several points overlapping and being too close to each other. The Poisson disk sampling does not have a problem with overlapping points but due to its random nature, the polygon is populated non-uniformly. The k-means method yields the best results with all points being distributed evenly across the whole polygon."};
    startCluster = mFAT.find_free_index();
    de3.init("thesis.txt", true, de3Content.size(), startCluster);
    mFAT.write_FAT(startCluster, de3Content.size());
    de2.mSize += de3.size();

    // writing to disk
    mFileStream.seekp(mBS.mDataStartAddress);
    mRootDir.write_to_disk(mFileStream);
    de1.write_to_disk(mFileStream);
    de2.write_to_disk(mFileStream);

    mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE * de1.mStartCluster);
    Utils::string_to_stream(mFileStream, de1Content);

    mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE * de2.mStartCluster);
    de3.write_to_disk(mFileStream);

    mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE * de3.mStartCluster);
    Utils::string_to_stream(mFileStream, de3Content);

    mFAT.write_to_disk(mFileStream);
}

std::vector<DirEntry> Filesystem::read_dir_entry_as_dir(const DirEntry& parentDir) {
    std::vector<DirEntry> result{};
    mFileStream.seekg(mBS.mDataStartAddress + CLUSTER_SIZE * parentDir.mStartCluster);

    DirEntry tmp;
    tmp.mount(mFileStream, mFileStream.tellg());

    while (tmp) {
        result.emplace_back(tmp);
        tmp.mount(mFileStream, mFileStream.tellg());
    }
    return result;
}

std::string Filesystem::read_dir_entry_as_file(const DirEntry& dirEntry) {
    if (!dirEntry.mIsFile) {
        std::cout << "File: " << Utils::remove_padding(dirEntry.mFilename) << "is a directory";
        return "";
    }
    std::string content{};

    uint idx = dirEntry.mStartCluster;
    uint readSize = dirEntry.mSize;
    do {
        mFileStream.seekp(mBS.mDataStartAddress + CLUSTER_SIZE * idx);
        if (readSize < CLUSTER_SIZE) {
            content += Utils::string_from_stream(mFileStream, readSize);
            break;
        }
        else {
            idx = mFAT.table[idx];

        }
    }
    while (true);
    return content;
}
