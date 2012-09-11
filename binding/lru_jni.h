#include "jni.h"

JNIEXPORT jint JNICALL Java_com_feinno_kv_MemDB_open(JNIEnv *, jobject, jlong);

JNIEXPORT jint JNICALL Java_com_feinno_kv_MemDB_set(JNIEnv *, jobject, jbyteArray, jint, jbyteArray, jint);

JNIEXPORT jint JNICALL Java_com_feinno_kv_MemDB_get(JNIEnv *, jobject, jbyteArray, jint, jbyteArray, jint);

JNIEXPORT jint JNICALL Java_com_feinno_kv_MemDB_remove(JNIEnv *, jobject, jbyteArray, jint);

JNIEXPORT void JNICALL Java_com_feinno_kv_MemDB_info(JNIEnv *, jobject , jbyteArray , jint);

JNIEXPORT void JNICALL Java_com_feinno_kv_MemDB_close(JNIEnv *, jobject);
