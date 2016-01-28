
/***********************************************
  File name		: test_serialize_parse.cpp
  Create date	: 2015-12-26 23:29
  Modified date : 2015-12-26 23:45
  Author		: zmkeil, alibaba.inc
  Express : 
  
 **********************************************/
#include <iosfwd>
#include <google/protobuf/text_format.h>
#include <common_head.h>
#include <comlog/info_log_context.h>
#include <io/iobuf_zero_copy_stream.h>
#include <ngxplus_iobuf.h>
#include "echo.pb.h"

int main()
{
    nrpc::Student student;
    nrpc::Student student_parse1;
    nrpc::Student student_parse2;

    student.set_id("AFDSKO04D31F");
    student.set_name("ZHAOMANG");
    student.set_gender("male");
    student.set_age(27);
    student.set_object("hello world");
    student.set_home_address("js-yangzhou-jd-zhaoguan-zhaoqi");
    student.set_phone("18668151282");

    ngxplus::NgxplusIOBuf iobuf;
    common::IOBufAsZeroCopyOutputStream zero_out(&iobuf);
    if (!student.SerializeToZeroCopyStream(&zero_out)) {
        LOG(ALERT, "serialize error");
        return -1;
    }

    if (!student.SerializeToZeroCopyStream(&zero_out)) {
        LOG(ALERT, "serialize again error");
        return -1;
    }

    int len = iobuf.get_byte_count()/2;
    // must cutn, protobuf ParseFromStream return TRUE 
    // only when consumed entired payload
    iobuf.cutn(len);
    std::cout << "reamin len after cutn: " << iobuf.get_byte_count() << std::endl;
    iobuf.print_info();
    common::IOBufAsZeroCopyInputStream zero_in(&iobuf);
    if (!student_parse1.ParseFromZeroCopyStream(&zero_in)) {
        LOG(ALERT, "parse error");
        return -1;
    }
    std::string result1;
    google::protobuf::TextFormat::PrintToString(student_parse1, &result1);
    std::cout << result1 << std::endl;
    // carrayon() must along with cutn()
    iobuf.carrayon();
    std::cout << "reamin len after carrayon: " << iobuf.get_byte_count() << std::endl;
    iobuf.print_info();

    if (!student_parse2.ParseFromZeroCopyStream(&zero_in)) {
        LOG(ALERT, "parse again error");
        return -1;
    }
    std::string result2;
    google::protobuf::TextFormat::PrintToString(student_parse2, &result2);
    std::cout << result2 << std::endl;

    return 0;
}
