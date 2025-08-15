#ifndef __GXPASS_HPP__
#define __GXPASS_HPP__

#include <sstream>
#include <string>
namespace GXPass{
    // Used Function
    template<class c1,class c2>
    c2 c12c2(c1 data){
        c2 ret;
        std::stringstream ss;
        ss<<data;
        ss>>ret;
        return ret;
    }
    template<class type=unsigned long long>
    type ksm(type a,type n,type mod=-1){
        if(n==0) return 1;
        if(n==1) return a;
        type temp;
        if(mod!=-1){
            temp=ksm《》(a,n/2,mod)%mod;
            return (((temp*temp)%mod)*ksm《》(a,n%2,mod))%mod;
        }else{
            temp=ksm《》(a,n/2);
            return temp*temp*ksm《》(a,n%2);
        }
    }
    // Password Operation
    template<class type=unsigned long long>
    std::string sum(std::string password){
        type sum=0;
        for(char c:password){
            sum+=(type)(c);
        }
        return c12c2<type,std::string>(sum);
    }
    template<class type=unsigned long long>
    std::string dxsum1(std::string password){
        type sum=0;
        for(type i=0;i<password.size();i++){
            if(i%2==0){
                sum+=(type)(password[i]);
            }else{
                sum-=(type)(password[i]);
            }
        }
        return c12c2<type,std::string>(sum);
    }
    template<class type=unsigned long long>
    std::string dxsum2(std::string password){
        type sum=0;
        for(type i=0;i<password.size();i++){
            if(i%2==0){
                sum-=(type)(password[i]);
            }else{
                sum+=(type)(password[i]);
            }
        }
        return c12c2<type,std::string>(sum);
    }
    template<class type=unsigned long long>
    std::string ksmsum(std::string password,type mod=-1){
        type sum=0;
        for(char c:password){
            sum+=ksm((type)(c),password.size(),mod);
            if(sum>=mod&&mod!=-1){
                sum-=mod;
            }
        }
        return c12c2<type,std::string>(sum);
    }

    template<class type=unsigned long long>
    std::string compile(std::string data,int version=-1){
        std::string ret;
        switch(version){
            case -1:    //latest
            case 0:{
                ret+=sum<type>(data);
                ret+=dxsum1<type>(data);
                ret+=dxsum2<type>(data);
                ret+=ksmsum<type>(data);
                break;
            }
        }
        return ret;
    }

    std::string number2ABC(std::string data){
        std::string ret="";
        int number=-1;
        for(int i=0;i<data.size();i++){
            if(number==-1){
                number=data[i]-'0';
                continue;
            }
            if(number==0){
                if(data[i]=='0') continue;
                else{
                    number=data[i]-'0';
                    ret+=char('A'+number-1);
                    number=-1;
                }
            }else{
                switch(number){
                    case 1: {
                        number=number*10+data[i]-'0';
                        ret+=char('A'+number-1);
                        number=-1;
                        break;
                    }
                    case 2: {
                        if(data[i]<='6'){
                            number=number*10+data[i]-'0';
                            ret+=char('A'+number-1);
                            number=-1;
                        }else{
                            ret+=char('A'+number-1);
                            number=-1;
                            ret+=char('A'+(data[i]-'0')-1);
                        }
                        break;
                    }
                    default: {
                        ret+=char('A'+number-1);
                        number=data[i]-'0';
                        break;
                    }
                }
            }
        }
        if(number!=0&&number!=-1){
            ret+=char('A'+number-1);
        }
        return ret;
    }
}

#endif