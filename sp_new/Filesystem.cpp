#include <ranges>
#include "Filesystem.hpp"

// Start : BootSector
void BootSector::init(uint diskSize) {
    mSignature = Utils::zero_padded_string("duclong", SIGNATURE_LEN);
    mDiskSize = diskSize;
    mClusterSize = CLUSTER_SIZE;
    mClusterCount = (mDiskSize - BootSector::SIZE()) / (CLUSTER_SIZE + sizeof(uint));
    mFatStartAddress = BootSector::SIZE();
    mDataStartAddress = BootSector::SIZE() + mClusterCount * sizeof(uint);

    DirEntry tmp;
    mMaxDirEntries = (CLUSTER_SIZE - sizeof(uint)) / tmp.SIZE();
    if (mMaxDirEntries == 0)
        throw std::runtime_error("Cluster size is too small. Try increasing it");
}

void BootSector::mount(std::fstream& stream, uint pos) {
    stream.seekg(pos);
    mSignature = Utils::string_from_stream(stream, SIGNATURE_LEN);
    mDiskSize = Utils::read_from_stream<uint>(stream);
    mClusterSize = Utils::read_from_stream<uint>(stream);
    mClusterCount = Utils::read_from_stream<uint>(stream);
    mFatStartAddress = Utils::read_from_stream<uint>(stream);
    mDataStartAddress = Utils::read_from_stream<uint>(stream);
    mMaxDirEntries = Utils::read_from_stream<uint>(stream);
}

void BootSector::write_to_disk(std::fstream &stream) {
    Utils::string_to_stream(stream, mSignature);
    Utils::write_to_stream(stream, mDiskSize, mClusterSize,mClusterCount,
                           mFatStartAddress, mDataStartAddress, mMaxDirEntries);
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

void FAT::write_FAT(uint idx, int fileSize) {
    if (fileSize < CLUSTER_SIZE)
        table[idx] = FAT::FLAG_FILE_END;
    else {
        int nextFreeCluster = find_free_index(idx);
        while (fileSize > CLUSTER_SIZE) {
            table[idx] = nextFreeCluster;
            idx = nextFreeCluster;
            nextFreeCluster = find_free_index(idx);
            fileSize -= CLUSTER_SIZE;
        }
        table[idx] = FAT::FLAG_FILE_END;
    }
}

void FAT::free_FAT(uint idx) {
    int nextCluster;
    do {
        nextCluster = table[idx];
        table[idx] = FAT::FLAG_UNUSED;
        idx = nextCluster;
    } while (nextCluster != FLAG_FILE_END);
}

int FAT::find_free_index(int ignoredIdx) const {
    // indexed range based for loop
    for (int idx = 0; const auto& it : *this) {
        if (it == FAT::FLAG_UNUSED && idx != ignoredIdx) return idx;
        else ++idx;
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

void DirEntry::mount(std::fstream &stream) {
    mFilename = Utils::string_from_stream(stream, FILENAME_LEN);
    mIsFile = Utils::read_from_stream<bool>(stream);
    mSize = Utils::read_from_stream<uint>(stream);
    mStartCluster = Utils::read_from_stream<uint>(stream);
}

void DirEntry::write_to_disk(std::fstream &stream) {
    Utils::string_to_stream(stream, mFilename);
    Utils::write_to_stream(stream, mIsFile, mSize, mStartCluster);
}

void DirEntry::write_content_to_disk(std::fstream &stream, uint dataStartAddress, const std::vector<uint>& clusters, const std::string& content) const {
    if (content.size() <= CLUSTER_SIZE) {
        stream.seekp(dataStartAddress + this->mStartCluster * CLUSTER_SIZE);
        Utils::string_to_stream(stream, content);
    }
    else {
        // splitting file content into cluster sized bites
        std::string part = content.substr(0, CLUSTER_SIZE);
        std::string rest = content.substr(CLUSTER_SIZE);
        for (auto cluster: clusters) {
            stream.seekp(dataStartAddress + cluster * CLUSTER_SIZE);
            Utils::string_to_stream(stream, part);

            if (rest.size() > CLUSTER_SIZE) {
                part = rest.substr(0, CLUSTER_SIZE);
                rest = rest.substr(CLUSTER_SIZE);
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

void Filesystem::wipe_all_clusters() {
    mFileStream.seekp(mBS.mDataStartAddress);
    auto clusterView = std::ranges::iota_view{0u, mBS.mClusterCount};
    for (const auto& _ : clusterView) {
        Utils::write_to_stream(mFileStream, mEmptyCluster);
    }
}

void Filesystem::init(uint size) {
    auto mode = std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc;
    mFileStream.open(mDiskName, mode);
    if (!mFileStream) {
        std::cout << "Error opening disk" << std::endl;
        exit(EXIT_FAILURE);
    }

    // construct disk sections
    mBS = BootSector();
    mFAT = FAT();
    mRootDir = DirEntry();

    // init disk sections
    mBS.init(size);
    mFAT.init(mBS.mClusterCount);
    mFAT.write_FAT(0, 0);
    mRootDir.init("/", false, 0, 0);
    DirEntry dot, dotdot;
    dot.init(".", false, 0, 0);         // '.' in root points to itself
    dotdot.init("..", false, 0, 0);     // '..' in root points to itself

    // saving info to disk
    mBS.write_to_disk(mFileStream);
    mFileStream.seekp(mBS.mFatStartAddress);
    mFAT.write_to_disk(mFileStream);
    Filesystem::wipe_all_clusters();

    // save rootDir info to disk
    mFileStream.seekp(mBS.mDataStartAddress);
    Utils::write_to_stream(mFileStream, mTwoDirEntries);
    dot.write_to_disk(mFileStream);
    dotdot.write_to_disk(mFileStream);

//    Filesystem::init_default_files();
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
}

DirEntry Filesystem::get_root_dir() {
    DirEntry rootDir;
    mFileStream.seekg(mBS.mDataStartAddress);
    Utils::read_from_stream<uint>(mFileStream);
    rootDir.mount(mFileStream);
    return rootDir;
}

DirEntry Filesystem::get_dir_entry(uint cluster, bool isFile, bool last) {
    DirEntry dirEntry;
    mFileStream.seekp(mBS.mDataStartAddress + (cluster * CLUSTER_SIZE));

    if (last) {     // this option is used if we want last dirEntry of directory
        auto dirEntryCount = Utils::read_from_stream<uint>(mFileStream);
        mFileStream.seekg((dirEntryCount - 1) * dirEntry.SIZE(), std::ios::cur);
        dirEntry.mount(mFileStream);
        return dirEntry;
    }
    if (!isFile) Utils::read_from_stream<uint>(mFileStream);    // if dir, skip offset
    dirEntry.mount(mFileStream);
    return dirEntry;
}

int Filesystem::get_position(const std::string& searched, const DirEntry& parent) {
    auto dirEntries = Filesystem::read_dir_entry_as_dir(parent);

    int i = 0;
    for (const auto& dirEntry : dirEntries) {
        if (Utils::remove_padding(dirEntry.mFilename) == searched) return i;
        else i++;
    }
    return -1;
}

uint Filesystem::get_child_dir_entry_count(const DirEntry &dirEntry) {
    if (dirEntry.mIsFile) throw std::runtime_error("Cannot get child dirEntries of a file");

    mFileStream.seekg(mBS.mDataStartAddress + dirEntry.mStartCluster * CLUSTER_SIZE);
    return Utils::read_from_stream<uint>(mFileStream);
}

std::optional<DirEntry> Filesystem::create_dir_entry(uint parentCluster, const std::string& name, bool isFile, const std::string& content) {
    // create new file
    DirEntry newDirEntry, dot, dotdot;
    uint startCluster = mFAT.find_free_index(-1);
    newDirEntry.init(name, isFile, content.size(), startCluster);
    dotdot = Filesystem::get_dir_entry(parentCluster, false, false);

    // check for existing filename in parent dir
    auto dirEntries = Filesystem::read_dir_entry_as_dir(dotdot);
    bool isDuplicate = std::any_of(dirEntries.begin(), dirEntries.end(), [&name](const DirEntry& dirEntry) {
        return Utils::remove_padding(dirEntry.mFilename) == name;
    });
    if (isDuplicate) {
        std::cout << name << " already exists" << std::endl;
        return std::nullopt;
    }

    // new dirEntry is a dir
    if (!isFile) {
        dot.init(".", isFile, 0, startCluster);
        dotdot.mFilename = Utils::zero_padded_string("..", FILENAME_LEN);

        mFileStream.seekp(mBS.mDataStartAddress + dot.mStartCluster * CLUSTER_SIZE);
        Utils::write_to_stream(mFileStream, mTwoDirEntries);
        dot.write_to_disk(mFileStream);
        dotdot.write_to_disk(mFileStream);
    }
    // set content of new file into FAT table
    mFAT.write_FAT(startCluster, content.size());
    mFileStream.seekp(mBS.mFatStartAddress);
    mFAT.write_to_disk(mFileStream);

    // save new info of parent dir to disk
    uint dirEntryCount = Filesystem::get_child_dir_entry_count(dotdot) + 1;
    mFileStream.seekp(mBS.mDataStartAddress + dotdot.mStartCluster * CLUSTER_SIZE);
    Utils::write_to_stream(mFileStream, dirEntryCount);

    // write new file meta-info as content of parent dir
    mFileStream.seekp((dirEntryCount - 1) * dotdot.SIZE(), std::ios::cur);
    newDirEntry.write_to_disk(mFileStream);

    // save new file content into disk
    auto clusters = Filesystem::get_cluster_locations(newDirEntry);
    newDirEntry.write_content_to_disk(mFileStream, mBS.mDataStartAddress, clusters, content);

    return newDirEntry;
}

std::optional<DirEntry> Filesystem::copy_dir_entry(uint parentCluster, const DirEntry& toCopy, const std::string& nameOfCopy) {
    std::string fileContent = Filesystem::read_dir_entry_as_file(toCopy);
    // copied file cannot be a directory and might already exist
    auto copiedDirEntry = Filesystem::create_dir_entry(parentCluster, nameOfCopy, toCopy.mIsFile, fileContent);

    if (!copiedDirEntry) return std::nullopt;
    else return copiedDirEntry;
}

void Filesystem::remove_dir_entry(uint parentCluster, uint position) {
    DirEntry toRemove;

    mFileStream.seekg(mBS.mDataStartAddress + parentCluster * CLUSTER_SIZE);
    auto dirEntryCount = Utils::read_from_stream<uint>(mFileStream);

    mFileStream.seekp(position * toRemove.SIZE(), std::ios::cur);
    toRemove.mount(mFileStream);

    DirEntry lastDirEntry = Filesystem::get_dir_entry(parentCluster, false, true);

    // check if dir has anything beside '.' and '..'
    if (!toRemove.mIsFile && Filesystem::get_child_dir_entry_count(toRemove) > mTwoDirEntries) {
        std::cout << "Directory " << Utils::remove_padding(toRemove.mFilename) << " is not empty" << std::endl;
        return;
    }

    // delete dirEntry content in all clusters
    auto clusters = Filesystem::get_cluster_locations(toRemove);
    for (auto cluster : clusters) {
        mFileStream.seekp(mBS.mDataStartAddress + cluster * CLUSTER_SIZE);
        Utils::write_to_stream(mFileStream, mEmptyCluster);
    }

    // free FAT table
    mFAT.free_FAT(toRemove.mStartCluster);
    mFileStream.seekp(mBS.mFatStartAddress);
    mFAT.write_to_disk(mFileStream);

    // change dirEntryCount and write the last entry of parent dir into the free space
    dirEntryCount--;
    mFileStream.seekp(mBS.mDataStartAddress + parentCluster * CLUSTER_SIZE);
    Utils::write_to_stream(mFileStream, dirEntryCount);
    mFileStream.seekp(position * toRemove.SIZE(), std::ios::cur);
    lastDirEntry.write_to_disk(mFileStream);
}

std::vector<uint> Filesystem::get_cluster_locations(const DirEntry& dirEntry) {
    std::vector<uint> clusters{dirEntry.mStartCluster};
    int nextCluster = mFAT.table[dirEntry.mStartCluster];

    while (nextCluster != FAT::FLAG_FILE_END) {
        clusters.push_back(nextCluster);
        nextCluster = mFAT.table[nextCluster];
    }

    return clusters;
}

void Filesystem::init_default_files() {
    Filesystem::create_dir_entry(0, "test.txt", true, "This is content of test.txt. Do what you want with this information. This is content of test.txt. Do what you want with this information.");  // cluster 1
    Filesystem::create_dir_entry(0, "home", false);     // cluster 2
    Filesystem::create_dir_entry(3, "thesis.txt", true, "As expected, the random sampling method has the worst result, with several points overlapping and being too close to each other. The Poisson disk sampling does not have a problem with overlapping points but due to its random nature, the polygon is populated non-uniformly. The k-means method yields the best results with all points being distributed evenly across the whole polygon.");
}

std::vector<DirEntry> Filesystem::read_dir_entry_as_dir(const DirEntry& dirEntry) {
    std::vector<DirEntry> result{};
    uint dirEntryCount = Filesystem::get_child_dir_entry_count(dirEntry);

    DirEntry tmp;
    while (dirEntryCount > 0) {
        tmp.mount(mFileStream);
        result.emplace_back(tmp);
        dirEntryCount--;
    }
    return result;
}

std::string Filesystem::read_dir_entry_as_file(const DirEntry& dirEntry) {
    std::string content{};

    int idx = dirEntry.mStartCluster;
    int readSize = dirEntry.mSize;
    do {
        mFileStream.seekp(mBS.mDataStartAddress + idx * CLUSTER_SIZE);
        if (readSize <= CLUSTER_SIZE) {
            content += Utils::string_from_stream(mFileStream, readSize);
            break;
        }
        else {
            content += Utils::string_from_stream(mFileStream, CLUSTER_SIZE);
            idx = mFAT.table[idx];
            readSize -= CLUSTER_SIZE;
        }
    } while (idx != FAT::FLAG_FILE_END);
    return content;
}