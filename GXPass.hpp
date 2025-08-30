// Version: 1.5.2    Latest Version: https://github.com/Necream/GXPass
#ifndef __GXPASS_HPP__
#define __GXPASS_HPP__

#include <sstream>
#include <string>
#include <limits>
#include <algorithm>

namespace GXPass {

    // 92个可打印字符，可自行修改
    std::string charset =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "!@#$%^&*()_+-=[]{}|;':\",./<>?";

    // 类型转换工具：c1类型转c2类型（如数字转字符串）
    template<class c1, class c2>
    c2 c12c2(const c1& data) {
        c2 ret;
        std::stringstream ss;
        ss << data;
        ss >> ret;
        return ret;
    }

    // 快速幂模板函数，默认模为类型最大值（表示无模）
    template<class T = unsigned long long>
    T ksm(T a, T n, T mod = static_cast<T>(-1)) {
        if (n == 0) return 1 % mod;
        if (n == 1) return a % mod;

        T temp = ksm<T>(a, n / 2, mod);
        temp = (temp * temp) % mod;

        if (n % 2 == 1) {
            temp = (temp * (a % mod)) % mod;
        }
        return temp;
    }

    // 计算字符串字符ASCII码和，返回字符串
    template<class type = unsigned long long>
    std::string sum(const std::string password) {
        type sum = 0;
        for (char c : password) {
            sum += static_cast<type>(c);
        }
        return c12c2<type, std::string>(sum);
    }

    // 奇偶位置加减和，返回字符串
    template<class type = unsigned long long>
    std::string dxsum1(const std::string password) {
        type sum = 0;
        for (type i = 0; i < static_cast<type>(password.size()); i++) {
            if (i % 2 == 0) {
                sum += static_cast<type>(password[i]);
            } else {
                sum -= static_cast<type>(password[i]);
            }
        }
        return c12c2<type, std::string>(sum);
    }

    // 另一种奇偶位置加减和，返回字符串
    template<class type = unsigned long long>
    std::string dxsum2(const std::string password) {
        type sum = 0;
        for (type i = 0; i < static_cast<type>(password.size()); i++) {
            if (i % 2 == 0) {
                sum -= static_cast<type>(password[i]);
            } else {
                sum += static_cast<type>(password[i]);
            }
        }
        return c12c2<type, std::string>(sum);
    }

    // 使用快速幂累加加密，模默认为类型最大值（无模）
    template<class type = unsigned long long>
    std::string ksmsum(const std::string password, type modulus = static_cast<type>(-1)) {
        type sum = 0;
        for (char c : password) {
            // 显式指定模板参数，确保推导正确
            sum = (sum + ksm<type>(static_cast<type>(c), static_cast<type>(password.size()), modulus)) % modulus;
        }
        return c12c2<type, std::string>(sum);
    }

    // 编译入口，调用各种密码运算  version:-1最新，0基础，1顺序重置
    template<class type = unsigned long long>
    std::string compile(const std::string data, int version = -1) {
        std::string ret;
        switch (version) {
            case -1: // latest
            case 2: {
                type sum_data=c12c2<std::string,type>(sum<type>(data));
                type dxsum1_data=c12c2<std::string,type>(dxsum1<type>(data));
                type dxsum2_data=c12c2<std::string,type>(dxsum2<type>(data));
                type ksmsum_data=c12c2<std::string,type>(ksmsum<type>(data));
                type sortdata[4]={sum_data,dxsum1_data,dxsum2_data,ksmsum_data};
                type data[4]={sum_data,dxsum1_data,dxsum2_data,ksmsum_data};
                std::sort(sortdata,sortdata+4);
                for(int i=0;i<4;i++){
                    for(int j=0;j<4;j++){
                        if(sortdata[i]==data[j]) ret+=c12c2<type,std::string>(data[j]);
                    }
                }
                break;
            }
            case 1: {
                ret += dxsum1<type>(data);
                ret += sum<type>(data);
                ret += dxsum2<type>(data);
                ret += ksmsum<type>(data);
                break;
            }
            case 0: {
                ret += sum<type>(data);
                ret += dxsum1<type>(data);
                ret += dxsum2<type>(data);
                ret += ksmsum<type>(data);
                break;
            }
            default:
                break;
        }
        return ret;
    }

    // 数字转字母映射辅助函数
    std::string number2ABC(const std::string data) {
        std::string ret = "";
        int number = -1;
        for (int i = 0; i < static_cast<int>(data.size()); i++) {
            if (number == -1) {
                number = data[i] - '0';
                continue;
            }
            if (number == 0) {
                if (data[i] == '0') continue;
                else {
                    number = data[i] - '0';
                    ret += char('A' + number - 1);
                    number = -1;
                }
            } else {
                switch (number) {
                    case 1: {
                        number = number * 10 + data[i] - '0';
                        ret += char('A' + number - 1);
                        number = -1;
                        break;
                    }
                    case 2: {
                        if (data[i] <= '6') {
                            number = number * 10 + data[i] - '0';
                            ret += char('A' + number - 1);
                            number = -1;
                        } else {
                            ret += char('A' + number - 1);
                            number = -1;
                            ret += char('A' + (data[i] - '0') - 1);
                        }
                        break;
                    }
                    default: {
                        ret += char('A' + number - 1);
                        number = data[i] - '0';
                        break;
                    }
                }
            }
        }
        if (number != 0 && number != -1) {
            ret += char('A' + number - 1);
        }
        return ret;
    }
    // 数字转安全字符映射辅助函数私有实现，不可以外部调用，防止错误使用
    std::string number2safestring_Private_CannotUse(std::string data, const std::string& chars = charset) {
        int charsLen=charset.size();
        std::string prefix="";
        std::string ret="";
        while(!data.empty()){
            prefix="";
            prefix=data.substr(0,2);
            int number=c12c2<std::string,int>(prefix);
            double rate = (double)(charsLen) / 100.0;
            int index=(int)(number*rate)%charsLen;
            // std::cout<<index<<" "<<chars[index]<<std::endl;
            ret+=chars[index];
            // std::cout<<ret<<std::endl;
            data.erase(0,2);
        }
        return ret;
    }
    // 数字转安全字符映射辅助函数（确定性扰动版）
    std::string number2safestring(std::string data, const std::string& chars = charset) {
        int dataLen = data.size();
        int charsLen = chars.size();
        std::string ret = "";

        // 如果长度为奇数，添加一个数字，使长度为偶数
        if (dataLen % 2 == 1) {
            int sum = 0;
            for (char c : data) sum += (c - '0');
            data += '0' + (sum % 10);
            dataLen++;
        }

        // 计算一个固定哈希值作为扰动基
        unsigned long long hash = 14695981039346656037ull; // FNV offset basis
        for (char c : data) {
            hash ^= static_cast<unsigned long long>(c);
            hash *= 1099511628211ull; // FNV prime
        }

        // 按两位一组映射到字符表
        for (int i = 0; i < dataLen; i += 2) {
            int num = (data[i] - '0') * 10 + (data[i + 1] - '0');
            // 添加确定性扰动：哈希混合
            int index = (num + (hash % charsLen)) % charsLen;
            ret += chars[index];
            // 更新哈希用于下一轮扰动
            hash = hash * 31 + index;
        }

        return ret;
    }

    // 计算安全密码长度，默认最大碰撞概率1e-6，字符集大小92
    int computeSafePassLen(int inputLength, int minPassLen = 6) {
        if(inputLength<minPassLen){
            return minPassLen/inputLength*1000;
        }else{
            return minPassLen*1000/inputLength+128;
        }
    }

    // Full safety Pass
    template<class type = unsigned long long>
    std::string fullsafe(const std::string data, int PassLen = 256, std::string chars = charset,int minPassLen = 6, int version = -1){ // 新增参数
        std::string Inputdata=data;
        for(int i=0;i<Inputdata.size();i++){
            Inputdata[i]=(Inputdata[i]+i)%127;
        }
        std::string OriginalPass=number2safestring(compile<type>(Inputdata, version), chars);
        if(PassLen==-1) PassLen=OriginalPass.size();
        if(PassLen==-2) PassLen=computeSafePassLen(data.size(),minPassLen);
        if(PassLen<=0) return "";
        std::string FinalPass="";
        FinalPass+=OriginalPass[0];
        std::string StepPass=OriginalPass;
        for(int i=1;i<PassLen;i++){
            StepPass=number2safestring(compile<type>(StepPass, version), chars);
            FinalPass+=StepPass[i%StepPass.size()];
        }
        return FinalPass;
    }


} // namespace GXPass

#endif // __GXPASS_HPP__
