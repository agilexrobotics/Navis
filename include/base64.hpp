#include <iostream>
#include <cstring>
#include <cstdlib>

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//判断字符是否为base64字符
inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

//base63 解码函数
uint8_t* base64_decode(const char* encoded_string, size_t& length) {
 
  size_t in_len = strlen(encoded_string);
  // 如果长度不是4的倍数，说明编码有误
  if (in_len % 4 != 0) {
    return nullptr;
  }

  // 解码后的数组大小
  size_t out_len = in_len / 4 * 3;

  // 如果编码末尾一个或两个字节用'='填充
  if (encoded_string[in_len - 1] == '=') {
    // 解码后的数组大小需要减少一位
    out_len--;
  }
  if (encoded_string[in_len - 2] == '=') {
    out_len--;
  }

  auto* decoded_data = new uint8_t[out_len];
  int i = 0;
  int j = 0;
  int in_ = 0;
  
  // 临时数组用于存储每四个base64字符对应的三个字节原始数据
  uint8_t  char_array_4[4], char_array_3[3];

  while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_];
    in_++;
    if (i == 4) {
      for (i = 0; i < 4; i++) {
        // 将base64字符映射到base64映射表中获取对应值
        char_array_4[i] = base64_chars.find(char_array_4[i]);
      }
      // 按照base64编码规则，将四个base64字符解析为三个字节原始数据
      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      // 将三个字节原始数据存储到解码后的数组中
      for (i = 0; i < 3; i++) {
        decoded_data[j++] = char_array_3[i];
      }
      i = 0;
    }
  }

  // 如果base64编码字符串长度不是4的倍数，末尾会用'='进行填充，在这里进行解码
  if (i) {
    for (int k = i; k < 4; k++) {
      char_array_4[k] = 0;
    }
    for (int k = 0; k < 4; k++) {
      char_array_4[k] = base64_chars.find(char_array_4[k]);
    }
    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (int k = 0; k < i - 1; k++) {
      decoded_data[j++] = char_array_3[k];
    }
  }

  //解码后数组的实际大小
  length = j;
  return decoded_data;
}
/*
int main() {
  const std::string encoded_str = "aGVsbG8gd29ybGQ=";
  size_t decoded_length = 0;
  auto decoded_data = base64_decode(encoded_str.c_str(), decoded_length);

  for (size_t i = 0; i < decoded_length; i++) {
    std::cout << decoded_data[i];
  }
  std::cout << std::endl;

  delete[] decoded_data;

  return 0;
}
*/
