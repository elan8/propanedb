#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <ostream>

#include <glog/logging.h>

#include "propanedb.grpc.pb.h"

// Read a file using using mmap(). Attempt to overlap reads of the file and writes by the user's code
// by reading the next segment 

class FileReader {
public:
    FileReader(const std::string& file_name, ::grpc::ServerWriter<::propane::PropaneBackupReply> *writer );
    //FileReader& operator=(FileReader&&);
    ~FileReader();

    // TODO: Provide some way to log non-critical errors which don't prevent the actual reading
    // of data, but could hurt performance

    // Read the file, calling OnChunkAvailable() whenever data are available. It blocks until the reading
    // is complete.
    void Read(size_t max_chunk_size);

    std::string GetFilePath() const
    {
        return m_file_path;
    }

    void OnChunkAvailable(const void* data, size_t size) ;

private:
    std::string m_file_path;
    std::unique_ptr< const std::uint8_t, std::function<void(const std::uint8_t*)> > m_data;
    size_t m_size;

    ::grpc::ServerWriter<::propane::PropaneBackupReply> *mp_writer;
    std::uint32_t m_id;

};