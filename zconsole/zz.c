// rdh 테스트
#include <qs.h>

int main(void)
{
	qn_runtime();

	QmVec4 s = qm_quat_rot_y(qm_d2rf(72));
	QmVec4 e = qm_quat_rot_y(qm_d2rf(74));
	qn_outputf("s: %f, %f, %f, %f", s.X, s.Y, s.Z, s.W);
	qn_outputf("e: %f, %f, %f, %f\n", e.X, e.Y, e.Z, e.W);
	for (int i = 0; i <= 100; i+=10)
	{
		QmVec4 q = qm_quat_slerp(s, e, i / 100.0f);
		qn_outputf("%d: %f, %f, %f, %f", i, q.X, q.Y, q.Z, q.W);
	}
	qn_outputs("");
	for (int i = 0; i <= 100; i += 10)
	{
		QmVec4 q = qm_quat_slerp_dxm(s, e, i / 100.0f);
		qn_outputf("%d: %f, %f, %f, %f", i, q.X, q.Y, q.Z, q.W);
	}

	return 0;
}
