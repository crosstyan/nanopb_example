#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include "msg.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

std::string uint8_to_hex_string(const uint8_t *v, const size_t s) {
  std::stringstream ss;

  ss << std::hex << std::setfill('0');

  for (int i = 0; i < s; i++) {
    ss << std::hex << std::setw(2) << static_cast<int>(v[i]);
  }

  return ss.str();
}

using PbEncodeCb = bool (*)(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

int main() {
  // dists should be ordered
  auto dists = std::vector<int>{0, 100, 200, 300, 400, 500, 600, 700, 800};
  auto speedes = std::vector<float>{1, 2, 2.5, 3, 4, 3.5, 3, 2, 4};

  if (dists.size() < speedes.size()){
    std::cout << "Error: dists size is too small. \n";
    return 1;
  }

  auto tuples = std::vector<TupleIntFloat>{};

  std::transform(
    dists.begin(), dists.end(), speedes.begin(),
    std::back_inserter(tuples),
    [](const auto &d, const auto &s){
      TupleIntFloat msg = TupleIntFloat_init_zero;
      msg.dist = d;
      msg.speed = s;
      return msg;
    }
  );

  TrackConfig config = TrackConfig_init_zero;
  uint8_t buf[256];
  pb_ostream_t stream = pb_ostream_from_buffer(buf, sizeof(buf));

  auto encode_tuple_list = [](pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
    // dereference it is ok since you don't write anything to this vector.
    auto &tuple_list = *(reinterpret_cast<std::vector<TupleIntFloat> *>(*arg));
    for (auto t : tuple_list){
      if (!pb_encode_tag_for_field(stream, field)) {
        return false;
      } else {
        bool res = pb_encode_submessage(stream, TupleIntFloat_fields, &t);
        if (!res){
          return false;
        }      
      }
    }
    return true;
  };

  config.lst.arg = reinterpret_cast<void *>(&tuples);
  config.lst.funcs.encode = encode_tuple_list;

  bool status = pb_encode(&stream, TrackConfig_fields, &config);
  if (status){
    auto message_length = stream.bytes_written;
    auto stream_hex = uint8_to_hex_string(buf, message_length);
    std::cout << stream_hex << "\n" << "length: " << message_length << "\n";
  } else {
    std::cout << "Error: No idea. What's going wrong? \n";
    return 1;
  }

  // it should be successfully encoded
  // Now let's decode it
  std::cout << "Start decoding\n";
  auto message_length = stream.bytes_written;
  auto recevied_tuple = std::vector<TupleIntFloat>{};
  pb_istream_t istream = pb_istream_from_buffer(buf, message_length);
  TrackConfig config_decoded = TrackConfig_init_zero;

  auto decode_tuple_list = [](pb_istream_t *stream, const pb_field_t *field, void **arg) {
    // Things will be weird if you dereference it
    // I don't know why though
    // auto tuple_list = *(reinterpret_cast<std::vector<TupleIntFloat> *>(*arg)); // not ok
    // https://stackoverflow.com/questions/1910712/dereference-vector-pointer-to-access-element
    // auto *tuple_list = (reinterpret_cast<std::vector<TupleIntFloat> *>(*arg)); // ok as pointer
    auto &tuple_list = *(reinterpret_cast<std::vector<TupleIntFloat> *>(*arg)); // ok as reference
    TupleIntFloat t = TupleIntFloat_init_zero;
    bool status = pb_decode(stream, TupleIntFloat_fields, &t);
    if (status) {
      tuple_list.emplace_back(t);
      return true;
    } else {
      return false;
    }
  };
  config_decoded.lst.arg = reinterpret_cast<void *>(&recevied_tuple);
  config_decoded.lst.funcs.decode = decode_tuple_list;
  bool status_decode = pb_decode(&istream, TrackConfig_fields, &config_decoded);
  std::cout << "Decode success? " << (status_decode ? "true" : "false") << "\n";
  if (status_decode){
    if (recevied_tuple.empty()){
      std::cout << "But I got nothing! \n";
      return 1;
    }
    for (auto t : recevied_tuple){
      std::cout << "dist: " << t.dist << "\tspeed: " << t.speed << "\n";
    }
    return 0;
  } else {
    std::cout << "Error: Something goes wrong when decoding \n";
    return 1;
  }
}