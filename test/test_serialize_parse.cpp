
/***********************************************
  File name		: test_serialize_parse.cpp
  Create date	: 2015-12-26 23:29
  Modified date : 2015-12-26 23:45
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/
#include <iosfwd>
#include <google/protobuf/text_format.h>
#include "common.h"
#include "info_log_context.h"
#include "iobuf_zero_copy_stream.h"
#include "echo.pb.h"

int main()
{
    nrpc::Student student;
    nrpc::Student student_parse;
    student.set_id("AFDSKO04D31F");
    student.set_name("ZHAOMANG");
    student.set_gender("male");
    student.set_age(27);
    student.set_object("hello world");
    student.set_home_address("js-yangzhou-jd-zhaoguan-zhaoqi");
    student.set_phone("18668151282");

    ngxplus::IOBuf iobuf;
    ngxplus::IOBufAsZeroCopyOutputStream zero_out(&iobuf);
    if (!student.SerializeToZeroCopyStream(&zero_out)) {
        LOG(NGX_LOG_LEVEL_ALERT, "serialize error");
        return -1;
    }

    ngxplus::IOBufAsZeroCopyInputStream zero_in(&iobuf);
    if (!student_parse.ParseFromZeroCopyStream(&zero_in)) {
        LOG(NGX_LOG_LEVEL_ALERT, "parse error");
        return -1;
    }

    std::string result;
    google::protobuf::TextFormat::PrintToString(student_parse, &result);
    std::cout << result << std::endl;
    return 0;
}
