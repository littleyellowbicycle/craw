#include "Precompile.h"
#include "BloomFilter.h"

BloomFilter::BloomFilter(void){
    bzero(m_bloomTable,sizeof(m_bloomTable));
}

bool BloomFilter::exist(string const& strUrl){
    int one=0;// 初始化1位计数器
    for(int i=0;i<HASH_FUNCS;i++){
        unsigned int bit=hash (i, strUrl )%(sizeof(m_bloomTable)*8);
        unsigned int idx=bit/(sizeof(m_bloomTable[0])*8);
        bit%=sizeof(m_bloomTable[0])*8;
        if(m_bloomTable[idx]&0x80000000>>bit)
            one++;
        else
            m_bloomTable[idx]|=0x80000000>>bit;
    }
    return one==HASH_FUNCS;
}


unsigned int BloomFilter:: hash (
		int           id,    ///< [in] 哈希算法标识号
		string const& strUrl ///< [in] 统一资源定位符
	)	const{
        unsigned in val;
        switch(id){
            case 0: val=times33(strUrl);break;
            case 1: val=timesnum(strUrl,31);break;
            case 2: val=aphash(strUrl);break;
            case 3: val=hash16777619(strUrl);break;
            case 4: val=mysqlhash(strUrl);break;
            case 5: val=timesnum(strUrl,32);break;
            case 6: val=timesnum(strUrl,131);break;
            case 7: val=timesnum(strUrl,1313);break;
            default:
                g_app->m_log.printf(Log::LEVEL_ERR, __FILE__, __LINE__,"无效哈希算法标识: %d", id);
        }

    }
