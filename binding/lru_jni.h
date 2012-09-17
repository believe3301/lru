#include "jni.h"

JNIEXPORT jlong JNICALL Java_com_feinno_kv_MemDB_open(JNIEnv *, jobject, jlong, jint);

JNIEXPORT jint JNICALL Java_com_feinno_kv_MemDB_set(JNIEnv *, jobject, jlong, jbyteArray, jint, jbyteArray, jint);

JNIEXPORT jint JNICALL Java_com_feinno_kv_MemDB_get(JNIEnv *, jobject, jlong, jbyteArray, jint, jbyteArray, jint);

JNIEXPORT jint JNICALL Java_com_feinno_kv_MemDB_remove(JNIEnv *, jobject, jlong, jbyteArray, jint);

JNIEXPORT void JNICALL Java_com_feinno_kv_MemDB_info(JNIEnv *, jobject , jlong, jbyteArray , jint);

JNIEXPORT void JNICALL Java_com_feinno_kv_MemDB_close(JNIEnv *, jobject, jlong);
