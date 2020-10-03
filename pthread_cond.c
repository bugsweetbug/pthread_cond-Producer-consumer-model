#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct msg
{
	int num;
	struct msg *next;
};

struct msg *head;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t has_data = PTHREAD_COND_INITIALIZER;

void err_thread(int ret, char *str)
{
	if (ret != 0)
	{
		fprintf(stderr, "%s:%s\n", str, strerror(ret));
		pthread_exit(NULL);
	}
}

void *producer(void *arg)
{
	while (1)
	{
		struct msg *mp = malloc(sizeof(struct msg));
		mp->num = rand() % 1000 + 1; //模拟生产数据
		pthread_mutex_lock(&mutex);
		mp->next = head; //往公共区域写数据
		head = mp;
		printf("----------producer:%d\n", mp->num);
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&has_data); //唤醒阻塞在条件变量 has_data上的线程

		sleep(rand() % 3);
	}
	return NULL;
}

void *customer(void *arg)
{
	while (1)
	{
		struct msg *mp;
		pthread_mutex_lock(&mutex);
		while (head == NULL) //如果头指针为空，公共区域没数据，用cond_wait等待
		{
			pthread_cond_wait(&has_data, &mutex); //阻塞等待条件变量
		}
		mp = head;
		head = mp->next;

		pthread_mutex_unlock(&mutex);
		printf("=====================customer id:%lu:%d\n", pthread_self(), mp->num);
		free(mp);
		sleep(rand() % 3);
	}
	return NULL;
}

int main()
{
	int ret;
	pthread_t pid, cid, cid2, cid3;

	srand(time(NULL));

	ret = pthread_create(&pid, NULL, producer, NULL); //生产者
	if (ret != 0)
		err_thread(ret, "pthread_create");
	ret = pthread_create(&cid, NULL, customer, NULL); //消费者
	if (ret != 0)
		err_thread(ret, "pthread_create2");
	ret = pthread_create(&cid2, NULL, customer, NULL); //消费者
	if (ret != 0)
		err_thread(ret, "pthread_create2");
	ret = pthread_create(&cid3, NULL, customer, NULL); //消费者
	if (ret != 0)
		err_thread(ret, "pthread_create2");

	pthread_join(pid, NULL);
	pthread_join(cid, NULL);
	pthread_join(cid2, NULL);
	pthread_join(cid3, NULL);

	return 0;
}
