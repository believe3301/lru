#include "lru_jni.h"
#include "lru.h"

/* open */
JNIEXPORT jint JNICALL 
Java_com_feinno_kv_MemDB_open(JNIEnv *jenv, jobject clazz, jlong bufsize)
{
    (void) clazz;
    (void) jenv;

    lru_init((size_t) bufsize);

    return (jint)0;
}

/* set */
JNIEXPORT jint JNICALL 
Java_com_feinno_kv_MemDB_set(JNIEnv *jenv, jobject clazz, jbyteArray jkey, jint jklen, jbyteArray jval, jint jvlen)
{
	(void)clazz;

	char *key, *val;
    int ret = -1;

	key = (char*)(*jenv)->GetByteArrayElements(jenv, jkey, 0);
	val = (char*)(*jenv)->GetByteArrayElements(jenv, jval, 0);

	if (key == NULL || val == NULL)
		goto RET;

	ret = item_set(key, (int)jklen, val, (int)jvlen);

RET:
	if (key) {
		(*jenv)->ReleaseByteArrayElements(jenv, jkey, (jbyte*)key, 0);
	}
	if (val){
		(*jenv)->ReleaseByteArrayElements(jenv, jval, (jbyte*)val, 0);
	}
	return ret;
}

/* get */
JNIEXPORT jint JNICALL 
Java_com_feinno_kv_MemDB_get(JNIEnv *jenv, jobject clazz, jbyteArray jkey, jint jklen, jbyteArray jbuf, jint jblen)
{
    (void) clazz;
	char *key, *buf;
    int ret = -1;

	key = (char*)(*jenv)->GetByteArrayElements(jenv, jkey, 0);
	buf = (char*)(*jenv)->GetByteArrayElements(jenv, jbuf, 0);

	if (key == NULL)
		goto RET;

    size_t sz;
	ret = item_get(key, (int)jklen, buf, (int)jblen, &sz);

    //(*jenv)->SetByteArrayRegion(jenv, jbuf, 0, sz, (jbyte*)buf);
RET:
	if (key) {
		(*jenv)->ReleaseByteArrayElements(jenv, jkey, (jbyte*)key, 0);
	}
	if (buf) {
		(*jenv)->ReleaseByteArrayElements(jenv, jbuf, (jbyte*)buf, 0);
	}

	return ret ? 0 : sz;
}

/* remove */
JNIEXPORT jint JNICALL 
Java_com_feinno_kv_MemDB_remove(JNIEnv *jenv, jobject clazz, jbyteArray jkey, jint jklen)
{
    (void) clazz;
	char *key;
    int ret = -1;

	key = (char*)(*jenv)->GetByteArrayElements(jenv, jkey, 0);

	if (key == NULL)
		goto RET;

	ret = item_delete(key, (int)jklen);

RET:
	if (key) {
		(*jenv)->ReleaseByteArrayElements(jenv, jkey, (jbyte*)key, 0);
	}
	return ret;
}


/* stat */
JNIEXPORT void JNICALL 
Java_com_feinno_kv_MemDB_info(JNIEnv *jenv, jobject clazz, jbyteArray jbuf, jint jblen)
{
    (void) clazz;
    char *buf;

    buf = (char*)(*jenv)->GetByteArrayElements(jenv, jbuf, 0);
    if (buf) {
        stat_print(buf, (int)jblen);
    }
}

/* free */
JNIEXPORT void JNICALL 
Java_com_feinno_kv_MemDB_close(JNIEnv *jenv, jobject clazz)
{
    (void) jenv;
    (void) clazz;

    lru_free();
}

