module;

export module int_encoder;


import byte_slice_reader;
import byte_slice_writer;

import std;
import type_alias;
import infinity_exception;
import new_pfordelta_compressor;

namespace infinity {



export template <typename T, typename Compressor>
class IntEncoder {
public:
    const static SizeT ENCODER_BUFFER_SIZE = 256;
    const static SizeT ENCODER_BUFFER_BYTE_SIZE = ENCODER_BUFFER_SIZE * sizeof(u32);

public:
    IntEncoder() {}
    virtual ~IntEncoder() {}

public:
    inline u32 Encode(ByteSliceWriter &slice_writer, const T *src, u32 src_len) const;

    inline u32 Decode(T *dest, u32 dest_len, ByteSliceReader &slice_reader) const;

private:
    Compressor compressor_;
};

/*
template <>
inline u32
IntEncoder<u32, SIMDBitPacking>::Encode(ByteSliceWriter& slice_writer,
                                             const u32* src,
                                             u32 src_len) const {
    u8 buffer[ENCODER_BUFFER_BYTE_SIZE];
    SizeT encode_len;
    encode_len = compressor_.Compress(src, src_len, (u32*)buffer, encode_len);
    slice_writer.Write((const u8*)&encode_len, sizeof(u8));
    slice_writer.Write((const u8*)buffer, encode_len);
    return encode_len;
}

template<>
inline u32
IntEncoder<u32, SIMDBitPacking>::Decode(u32* dest, u32 dest_len, ByteSliceReader& slice_reader) const {
    u8 buffer[ENCODER_BUFFER_BYTE_SIZE];
    u32 comp_len = slice_reader.ReadByte();
    void* buf_ptr = buffer;
    SizeT len = slice_reader.ReadMayCopy(buf_ptr, comp_len);
    if(len != comp_len) {
        Error<StorageException>("Decode posting FAILEDF");
    }
    SizeT destlen = dest_len;
    compressor_.Decompress((const u32*)buf_ptr, comp_len, dest, destlen);
    return destlen;
}
*/

template <typename T, typename Compressor>
inline u32 IntEncoder<T, Compressor>::Encode(ByteSliceWriter &slice_writer, const T *src, u32 src_len) const {
    u8 buffer[ENCODER_BUFFER_BYTE_SIZE];
    u32 len;
    len = compressor_.Compress((u32 *)buffer, ENCODER_BUFFER_SIZE, src, src_len, true);
    u32 encode_len = len * sizeof(u32);
    slice_writer.Write((const u8 *)buffer, encode_len);
    return encode_len;
}

template <typename T, typename Compressor>
inline u32 IntEncoder<T, Compressor>::Decode(T *dest, u32 dest_len, ByteSliceReader &slice_reader) const {
    u8 buffer[ENCODER_BUFFER_BYTE_SIZE];
    u32 header = (u32)slice_reader.PeekInt32();
    SizeT comp_len = compressor_.GetCompressedLength(header) * sizeof(u32);
    if(comp_len > ENCODER_BUFFER_BYTE_SIZE) {
        Error<StorageException>("IntEncoder: error");
    }
    void *buf_ptr = buffer;
    SizeT len = slice_reader.ReadMayCopy(buf_ptr, comp_len);
    if (len != comp_len) {
        Error<StorageException>("Decode posting FAILEDF");
    }
    return (u32)compressor_.Decompress(dest, dest_len, (const u32 *)buf_ptr, comp_len);
}

} // namespace infinity
