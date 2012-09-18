#include "lru_jni.h"
#include "lru.h"

/* open */
JNIEXPORT jlong JNICALL 
Java_com_feinno_kv_MemDB_open(JNIEnv *jenv, jobject clazz, jlong bufsize, jint hashpower)
{
    (void) clazz;
    (void) jenv;

    lru *l = lru_init((uint64_t) bufsize, (unsigned int) hashpower);
    return (jlong)(intptr_t)l;
}

/* set */
JNIEXPORT jint JNICALL 
Java_com_feinno_kv_MemDB_set(JNIEnv *jenv, jobject clazz, jlong ptr, jbyteArray jkey, jint jklen, jbyteArray jval, jint jvlen)
{
	(void)clazz;

	char *key, *val;
    int ret = -1;
    lru *l = (lru*) ptr;

	key = (char*)(*jenv)->GetByteArrayElements(jenv, jkey, 0);
	val = (char*)(*jenv)->GetByteArrayElements(jenv, jval, 0);

	if (key == NULL || val == NULL)
		goto RET;

	ret = item_set(l, key, (int)jklen, val, (int)jvlen);

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
Java_com_feinno_kv_MemDB_get(JNIEnv *jenv, jobject clazz, jlong ptr, jbyteArray jkey, jint jklen, jbyteArray jbuf, jint jblen)
{
    (void) clazz;
	char *key, *buf;
    int ret = -1;
    lru *l = (lru*) ptr;

	key = (char*)(*jenv)->GetByteArrayElements(jenv, jkey, 0);
	buf = (char*)(*jenv)->GetByteArrayElements(jenv, jbuf, 0);

	if (key == NULL)
		goto RET;

    size_t sz;
	ret = item_get(l, key, (int)jklen, buf, (int)jblen, &sz);

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
Java_com_feinno_kv_MemDB_remove(JNIEnv *jenv, jobject clazz, jlong ptr, jbyteArray jkey, jint jklen)
{
    (void) clazz;
	char *key;
    int ret = -1;
    lru *l = (lru*) ptr;

	key = (char*)(*jenv)->GetByteArrayElements(jenv, jkey, 0);

	if (key == NULL)
		goto RET;

	ret = item_delete(l, key, (int)jklen);

RET:
	if (key) {
		(*jenv)->ReleaseByteArrayElements(jenv, jkey, (jbyte*)key, 0);
	}
	return ret;
}


/* stat */
JNIEXPORT void JNICALL 
Java_com_feinno_kv_MemDB_info(JNIEnv *jenv, jobject clazz, jlong ptr, jbyteArray jbuf, jint jblen)
{
    (void) clazz;
    char *buf;
    lru *l = (lru*) ptr;

    buf = (char*)(*jenv)->GetByteArrayElements(jenv, jbuf, 0);
    if (buf) {
        stat_print(l, buf, (int)jblen);
		(*jenv)->ReleaseByteArrayElements(jenv, jbuf, (jbyte*)buf, 0);
    }
}

/* free */
JNIEXPORT void JNICALL 
Java_com_feinno_kv_MemDB_close(JNIEnv *jenv, jobject clazz, jlong ptr)
{
    (void) jenv;
    (void) clazz;
    lru *l = (lru*) ptr;

    lru_free(l);
}

