/*
* 语音识别（Automatic Speech Recognition）技术能够从语音中识别出特定的命令词或语句模式。
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "record.h"

#define	BUFFER_SIZE 2048
#define HINTS_SIZE  100
#define GRAMID_LEN	128
#define FRAME_LEN	640

int i;
int tmp;
char stmp[12];
char *final;

//0~9
char s0[] = {-23,-101,-74};
char s1[] = {-28,-72,-128};
char s2[] = {-28,-70,-116};
char s3[] = {-28,-72,-119};
char s4[] = {-27,-101,-101};
char s5[] = {-28,-70,-108};
char s6[] = {-27,-123,-83};
char s7[] = {-28,-72,-125};
char s8[] = {-27,-123,-85};
char s9[] = {-28,-71,-99};
//王K、后Q、象B、马N、车R、兵P
char sk[] = {-25,-114,-117};
char sq[] = {-27,-112,-114};
char sb[] = {-24,-79,-95};
char sn[] = {-23,-87,-84};
char sr[] = {-24,-67,-90};
char sp[] = {-27,-123,-75};
//进F、平T、退E
char sf[] = {-24,-65,-101};
char st[] = {-27,-71,-77};
char se[] = {-23,-128,-128};

int coms(const char* dest, const char* src, int n)
{
    int i;
    for(i=0; i<n; i++){
        if(dest[i]=='\0' || src[i]=='\0')
            return -2;
        else{
            if(dest[i] > src[i])
                return 1;
            else if(dest[i] < src[i])
                return -1;
            else if(dest[i] == src[i])
                continue;
        }
    }
    return 0;
}

char* match_chinese(char* src)
{
    char tmp[3];
    char *dest;
    dest = (char *)malloc(5*sizeof(char));
    dest[4] = '\0';

    strncpy(tmp, src, 3);

    if(coms(tmp,sk,3) == 0)
        dest[0] = '6';
    else if(coms(tmp,sq,3) == 0)
        dest[0] = '5';
    else if(coms(tmp,sb,3) == 0)
        dest[0] = '3';
    else if(coms(tmp,sn,3) == 0)
        dest[0] = '2';
    else if(coms(tmp,sr,3) == 0)
        dest[0] = '4';
    else if(coms(tmp,sp,3) == 0)
        dest[0] = '1';

    strncpy(tmp, src+3, 3);
    if(coms(tmp,s0,3) == 0)
        dest[1] = '0';
    else if(coms(tmp,s1,3) == 0)
        dest[1] = '1';
    else if(coms(tmp,s2,3) == 0)
        dest[1] = '2';
    else if(coms(tmp,s3,3) == 0)
        dest[1] = '3';
    else if(coms(tmp,s4,3) == 0)
        dest[1] = '4';
    else if(coms(tmp,s5,3) == 0)
        dest[1] = '5';
    else if(coms(tmp,s6,3) == 0)
        dest[1] = '6';
    else if(coms(tmp,s7,3) == 0)
        dest[1] = '7';
    else if(coms(tmp,s8,3) == 0)
        dest[1] = '8';
    else if(coms(tmp,s9,3) == 0)
        dest[1] = '9';

    strncpy(tmp, src+6, 3);
    if(coms(tmp,sf,3) == 0)
        dest[2] = 'F';
    else if(coms(tmp,st,3) == 0)
        dest[2] = 'T';
    else if(coms(tmp,se,3) == 0)
        dest[2] = 'E';

    strncpy(tmp, src+9, 3);
    if(coms(tmp,s0,3) == 0)
        dest[3] = '0';
    else if(coms(tmp,s1,3) == 0)
        dest[3] = '1';
    else if(coms(tmp,s2,3) == 0)
        dest[3] = '2';
    else if(coms(tmp,s3,3) == 0)
        dest[3] = '3';
    else if(coms(tmp,s4,3) == 0)
        dest[3] = '4';
    else if(coms(tmp,s5,3) == 0)
        dest[3] = '5';
    else if(coms(tmp,s6,3) == 0)
        dest[3] = '6';
    else if(coms(tmp,s7,3) == 0)
        dest[3] = '7';
    else if(coms(tmp,s8,3) == 0)
        dest[3] = '8';
    else if(coms(tmp,s9,3) == 0)
        dest[3] = '9';

    return dest;
}

int look_level(const char* src)
{
    if(strlen(src) <=44 )
        return 0;
    else if(strlen(src) <= 88)
        return ((src[11]-48)*10 + src[12]-48) > ((src[11+44]-48)*10 + src[12+44]-48) ? 0:1;
    else if(strlen(src) <= 132)
    {
        int temp1 = ((src[11]-48)*10 + src[12]-48);
        int temp2 = ((src[11+44]-48)*10 + src[12+44]-48);
        int temp3 = ((src[11+44+44]-48)*10 + src[12+44+44]-48);
        if(temp1 >= temp2)
            if(temp1 >= temp3)
                return 0;
            else
                return 2;
        else
            if(temp2 >= temp3)
                return 1;
            else
                return 2;
    }
    else
        return 3;
}

int get_grammar_id(char* grammar_id, unsigned int id_len)
{
	FILE*			fp				=	NULL;
	char*			grammar			=	NULL;
	unsigned int	grammar_len		=	0;
	unsigned int	read_len		=	0;
	const char*		ret_id			=	NULL;
	unsigned int	ret_id_len		=	0;
	int				ret				=	-1;

	if (NULL == grammar_id)
		goto grammar_exit;

	fp = fopen("gm_continuous_digit.abnf", "rb");
	if (NULL == fp)
	{
		printf("\nopen grammar file failed!\n");
		goto grammar_exit;
	}

	fseek(fp, 0, SEEK_END);
	grammar_len = ftell(fp); //获取语法文件大小
	fseek(fp, 0, SEEK_SET);

	grammar = (char*)malloc(grammar_len + 1);
	if (NULL == grammar)
	{
		printf("\nout of memory!\n");
		goto grammar_exit;
	}

	read_len = fread((void *)grammar, 1, grammar_len, fp); //读取语法内容
	if (read_len != grammar_len)
	{
		printf("\nread grammar error!\n");
		goto grammar_exit;
	}
	grammar[grammar_len] = '\0';

	ret_id = MSPUploadData("usergram", grammar, grammar_len, "dtt = abnf, sub = asr", &ret); //上传语法
	if (MSP_SUCCESS != ret)
	{
		printf("\nMSPUploadData failed, error code: %d.\n", ret);
		goto grammar_exit;
	}

	ret_id_len = strlen(ret_id);
	if (ret_id_len >= id_len)
	{
		printf("\nno enough buffer for grammar_id!\n");
		goto grammar_exit;
	}
	strncpy(grammar_id, ret_id, ret_id_len);
	//printf("grammar_id: \"%s\" \n", grammar_id); //下次可以直接使用该ID，不必重复上传语法。

grammar_exit:
	if (NULL != fp)
	{
		fclose(fp);
		fp = NULL;
	}
	if (NULL!= grammar)
	{
		free(grammar);
		grammar = NULL;
	}
	return ret;
}

void run_asr(const char* audio_file, const char* params, char* grammar_id)
{
	const char*		session_id						= NULL;
	char			rec_result[BUFFER_SIZE]		 	= {'\0'};
	char			hints[HINTS_SIZE]				= {'\0'}; //hints为结束本次会话的原因描述，由用户自定义
	unsigned int	total_len						= 0;
	int 			aud_stat 						= MSP_AUDIO_SAMPLE_CONTINUE;		//音频状态
	int 			ep_stat 						= MSP_EP_LOOKING_FOR_SPEECH;		//端点检测
	int 			rec_stat 						= MSP_REC_STATUS_SUCCESS;			//识别状态
	int 			errcode 						= MSP_SUCCESS;

	FILE*			f_pcm 							= NULL;
	char*			p_pcm 							= NULL;
	long 			pcm_count 						= 0;
	long 			pcm_size 						= 0;
	long			read_size						= 0;

	if (NULL == audio_file)
		goto asr_exit;

	f_pcm = fopen(audio_file, "rb");
	if (NULL == f_pcm)
	{
		printf("\nopen [%s] failed!\n", audio_file);
		goto asr_exit;
	}

	fseek(f_pcm, 0, SEEK_END);
	pcm_size = ftell(f_pcm); //获取音频文件大小
	fseek(f_pcm, 0, SEEK_SET);

	p_pcm = (char*)malloc(pcm_size);
	if (NULL == p_pcm)
	{
		printf("\nout of memory!\n");
		goto asr_exit;
	}

	read_size = fread((void *)p_pcm, 1, pcm_size, f_pcm); //读取音频文件内容
	if (read_size != pcm_size)
	{
		printf("\nread [%s] failed!\n", audio_file);
		goto asr_exit;
	}

	//printf("\n开始语音识别 ...\n");
	session_id = QISRSessionBegin(grammar_id, params, &errcode);
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQISRSessionBegin failed, error code:%d\n", errcode);
		goto asr_exit;
	}

	while (1)
	{
		unsigned int len = 10 * FRAME_LEN; // 每次写入200ms音频(16k，16bit)：1帧音频20ms，10帧=200ms。16k采样率的16位音频，一帧的大小为640Byte
		int ret = 0;

		if (pcm_size < 2 * len)
			len = pcm_size;
		if (len <= 0)
			break;

		aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
		if (0 == pcm_count)
			aud_stat = MSP_AUDIO_SAMPLE_FIRST;

		//printf(">");
		ret = QISRAudioWrite(session_id, (const void *)&p_pcm[pcm_count], len, aud_stat, &ep_stat, &rec_stat);
		if (MSP_SUCCESS != ret)
		{
			printf("\nQISRAudioWrite failed, error code:%d\n",ret);
			goto asr_exit;
		}

		pcm_count += (long)len;
		pcm_size  -= (long)len;

		if (MSP_EP_AFTER_SPEECH == ep_stat)
			break;
		usleep(200*1000); //模拟人说话时间间隙，10帧的音频长度为200ms
	}
	errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQISRAudioWrite failed, error code:%d\n",errcode);
		goto asr_exit;
	}

	while (MSP_REC_STATUS_COMPLETE != rec_stat)
	{
		const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
		if (MSP_SUCCESS != errcode)
		{
			printf("\nQISRGetResult failed, error code: %d\n", errcode);
			goto asr_exit;
		}
		if (NULL != rslt)
		{
			unsigned int rslt_len = strlen(rslt);
			total_len += rslt_len;
			if (total_len >= BUFFER_SIZE)
			{
				printf("\nno enough buffer for rec_result !\n");
				goto asr_exit;
			}
			strncat(rec_result, rslt, rslt_len);
		}
		usleep(150*1000); //防止频繁占用CPU
	}
	//printf("\n语音识别结束\n");
	/*printf("=============================================================\n");
	printf("%s\n\n",rec_result);
    for(i=30; i<44; i++){
        printf("%d* ",rec_result[i]);
    }
    printf("\n");

    for(i=30; i<44; i++){
        printf("%d* ",rec_result[i+44]);
    }
    printf("\n");

     for(i=30; i<44; i++){
        printf("%d* ",rec_result[i+44+44]);
    }
    printf("\n");

	printf("=============================================================\n");
*/
    //printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
    tmp = look_level(rec_result);
    if(tmp >= 3)
        printf("WTF!!!");
    else{
        for(i=0; i<12; i++){
            stmp[i] = rec_result[30+tmp*44+i];
            //printf("%c",rec_result[30+tmp*44+i]);
        }
    }
    //printf("\n\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");

    //printf("\n#################################################################\n");
    //("%s\n\n",stmp);
    final = match_chinese(stmp);
    printf("%s",final);
    //for(i=0; final[i]!='\0'&&i<12; i++)
        //printf("%d ",final[i]);
    //printf("\n#################################################################\n");
asr_exit:
	if (NULL != f_pcm)
	{
		fclose(f_pcm);
		f_pcm = NULL;
	}
	if (NULL != p_pcm)
	{
		free(p_pcm);
		p_pcm = NULL;
	}

	QISRSessionEnd(session_id, hints);
}

int main(int argc, char* argv[])
{
	int			ret						=	MSP_SUCCESS;
	const char* login_params			=	"appid = 576cd407, work_dir = ."; //登录参数,appid与msc库绑定,请勿随意改动
	/*
	* sub:             请求业务类型
	* result_type:     识别结果格式
	* result_encoding: 结果编码格式
	*
	* 详细参数说明请参阅《iFlytek MSC Reference Manual》
	*/
	const char*	session_begin_params	=	"sub = asr, result_type = plain, result_encoding = utf8";
	char*		grammar_id				=	NULL;

	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed, error code: %d.\n",ret);
		goto exit; //登录失败，退出登录
	}

	//printf("开始录音\n");
  startRecord("default");
  //printf("\n录音结束\n\n");

	//printf("开始识别\n\n");

	grammar_id = (char*)malloc(GRAMID_LEN);
	if (NULL == grammar_id)
	{
		printf("out of memory !\n");
		goto exit;
	}
	memset(grammar_id, 0, GRAMID_LEN);

	//printf("上传语法 ...\n");
	ret = get_grammar_id(grammar_id, GRAMID_LEN);
	if (MSP_SUCCESS != ret)
		goto exit;
	//printf("上传语法成功\n");

 	run_asr("wav/test2.wav", session_begin_params, grammar_id); //iflytek01对应的音频内容：“18012345678”

exit:
	if (NULL != grammar_id)
	{
		free(grammar_id);
		grammar_id = NULL;
	}
	//printf("按任意键退出 ...\n");
	//getchar();
	MSPLogout(); //退出登录

	return 0;
}
