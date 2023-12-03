#include <qn.h>

int main()
{
    int v[4];
    qn_runtime(v);
    printf("버전: %d.%d.%d.%d\n", v[0], v[1], v[2], v[3]);

    qnDateTime dt;
    qn_now(&dt);
    printf("시간: %02d:%02d:%02d.%03d\n", dt.hour, dt.minute, dt.second, dt.millisecond);

    char* str;
    int len=qn_asprintf(&str, "이것은 숫자:%d, 실수:%f, 문자열:%s", 12, 12.345, "문자열");
    printf("길이: %d, 내용:%s\n", len, str);
    qn_free(str);

    return 0;
}
