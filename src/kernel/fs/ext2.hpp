#pragma once

#include "fs.hpp"

#include "../algo/Bitmap.hpp"
#include "../algo/String.hpp"
#include "../algo/Vector.hpp"
#include "../drivers/disk/Ata.hpp"
#include "../drivers/disk/DiskDriver.hpp"
#include "../types.hpp"

#define EXT2_STATE_CLEAN 1
#define EXT2_STATE_ERRORS 2

#define EXT2_ERROR_HANDLING_IGNORE 1
#define EXT2_ERROR_HANDLING_REMOUNT_R_ONLY 2
#define EXT2_ERROR_HANDLING_KERNEL_PANIK 3

typedef struct {
    uint32_t inodes_count; // Total number of inodes in file system
    uint32_t blocks_count; // Total number of blocks in file system
    uint32_t superblock_blocks_count; // Number of blocks reserved for superuser
    uint32_t unallocated_blocks_count; // Total number of unallocated blocks
    uint32_t unallocated_inodes_count; //Total number of unallocated inodes
    uint32_t superblock_block_number; // Block number of the block containing the superblock
    uint32_t block_shift; // log2 (block size) - 10. (In other words, the number to shift 1,024 to the left by to obtain the block size)
    uint32_t fragment_shift; // log2 (fragment size) - 10. (In other words, the number to shift 1,024 to the left by to obtain the fragment size)
    uint32_t blocks_per_block_group; // Number of blocks in each block group
    uint32_t fragments_per_block_group; // Number of fragments in each block group
    uint32_t inodes_per_block_group; // Number of inodes in each block group
    uint32_t last_mount_time; // in POSIX time
    uint32_t last_written_time; // in POSIX time

    uint16_t mounted_since_consistency_check; // Number of times the volume has been mounted since its last consistency check
    uint16_t mounted_since_consistency_check_allowed; // Number of mounts allowed before a consistency check (fsck) must be done
    uint16_t magic; // Ext2 signature (0xef53)
    uint16_t file_system_state; // File system state
    uint16_t error_handling_method; // What to do when an error is detected
    uint16_t version_low; // low portion of the version

    uint32_t consistency_check_time; // Time of the last consistency check (in POSIX)
    uint32_t consistency_check_interval; // Interval between forced consistency checks
    uint32_t os_id; // Operating system ID from which the filesystem on this volume was created
    uint32_t version_high; // high portion of the version

    uint16_t reserved_blocks_user_id; // User ID that can use reserved blocks
    uint16_t reserved_blocks_group_id; // Group ID that can use reserved blocks

} __attribute__((packed)) __attribute__((aligned(1024))) ext2_superblock_t;

typedef struct {
    uint32_t block_bitmap_addr; // Block address of block usage bitmap
    uint32_t inode_bitmap_addr; // Block address of inode usage bitmap
    uint32_t inode_table_addr; // Starting block address of inode table
    uint16_t unallocated_block_count; // Number of unallocated blocks in group
    uint16_t unallocated_inodes_count; // Number of unallocated inodes in group
    uint16_t directories_count; // Number of directories in group
    uint64_t unused1;
    uint32_t unused2;
    uint16_t unused3;
} __attribute__((packed)) block_group_descriptor_t;

#define FIFO 0x1000
#define CHAR_DEVICE 0x2000
#define DIRECTORY 0x4000
#define BLOCK_DEVICE 0x6000
#define FILE 0x8000 // 81A4
#define SYMLINK 0xA000
#define SOCKET 0xC000

#define EXECUTE 0x001
#define WRITE 0x002
#define READ 0x004
#define G_EXECUTE 0x008
#define G_WRITE 0x010
#define G_READ 0x020
#define U_EXECUTE 0x040
#define U_WRITE 0x080
#define U_READ 0x100
#define STICKY_BIT 0x200
#define SET_G_ID 0x400
#define SET_U_ID 0x800

typedef struct {
    uint16_t type_and_permissions;
    uint16_t user_id;
    uint32_t size; // in bytes
    uint32_t a_time; // last access time
    uint32_t c_time; // creation time
    uint32_t m_time; // last modification time
    uint32_t d_time; // deletion time
    uint16_t group_id;
    uint16_t h_links_count; // Count of hard links (directory entries) to this inode
    uint32_t disk_sectors_count;
    uint32_t flags;
    uint32_t os_value_1;
    uint32_t direct_block_pointers[12];
    uint32_t singly_inderect_block_pointer;
    uint32_t doubly_inderect_block_pointer;
    uint32_t triply_inderect_block_pointer;
    uint32_t gen_number;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t fragment_block_adress;
    uint32_t os_val_2_low;
    uint32_t os_val_2_mid;
    uint32_t os_val_2_high;
} __attribute__((packed)) inode_t;

#define DE_UNKNOWN 0
#define DE_REGULAR 1
#define DE_DIRECTORY 2
#define DE_CHARACTED_DEVICE 3
#define DE_BLOCK_DEVICE 4
#define DE_FIFO 5
#define DE_SOCKET 6
#define DE_SYMBOLIC_LINK 7

typedef struct
{
    uint32_t inode;
    uint16_t size;
    uint8_t name_len_low;
    uint8_t type_indicator;
    char name_characters;
} __attribute__((packed)) dir_entry_t;

namespace kernel::fs::ext2 {
using algorithms::Bitmap;
using algorithms::String;
using algorithms::Vector;

struct inode_cache_t {
    uint32_t inode;
    inode_t inode_struct;
};

class Ext2 : public FS {
public:
    Ext2(drivers::DiskDriver&);
    ~Ext2();

    bool init();

    File& root() override { return m_root; }

    // file system api functions
    uint32_t read(const File& file, uint32_t offset, uint32_t size, void* buffer) override;
    uint32_t write(const File& file, uint32_t offset, uint32_t size, void* buffer) override;
    File* finddir(const File& directory, const String& filename) override;
    Vector<File*> listdir(const File& directory) override;
    File& create(const File& directory, File& file) override;
    bool erase(const File& directory, const File& file) override;

    // test func
    void read_directory(uint32_t inode);
    void read_inode(uint32_t inode);

private:
    drivers::DiskDriver& m_disk_driver;

    // root
    File m_root;

    // file system params
    ext2_superblock_t m_superblock;
    block_group_descriptor_t* m_bgd_table {};
    uint32_t m_block_size;
    uint32_t m_bgd_table_size;

    // buffers
    char* m_block_buffer {};
    char* m_table_buffer_1 {};
    char* m_table_buffer_2 {};
    char* m_table_buffer_3 {};

    void bind_fs(File& file)
    {
        file.bind_fs(this);
    }

    // driver based
    bool read_blocks(uint32_t block, uint32_t block_size, void* mem);
    bool read_block(uint32_t block, void* mem);
    bool write_blocks(uint32_t block, uint32_t block_size, void* mem);
    bool write_block(uint32_t block, void* mem);

    // inode helpers
    inode_t get_inode_structure(uint32_t inode);
    bool save_inode_structure(inode_cache_t* inode);
    uint32_t resolve_inode_local_block(inode_cache_t* inode, uint32_t block, bool need_create = false);
    uint32_t read_inode_content(inode_cache_t* inode, uint32_t offset, uint32_t size, void* mem);
    uint32_t write_inode_content(inode_cache_t* inode, uint32_t offset, uint32_t size, void* mem);
    uint32_t occypy_inode(uint32_t preferd_block_group = 0);
    bool free_inode(uint32_t inode);

    // block helpers
    uint32_t occypy_block(uint32_t preferd_block_group = 0, bool fill_zeroes = false);
};
}