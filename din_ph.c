#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "din_ph.h"

#define N_PHILOSOPHER 5
static philosopher phils[N_PHILOSOPHER];
static spoon spoons[N_PHILOSOPHER];

/*
 * Indent the output of philosopher information based on
 * philosopher's phi_id. Make any print output more readable
 * for any threads.
 */
static void
phil_print_indent(philosopher *phil){
    int i;

    for (i = 0; i < phil->phil_id; i++){
	printf("\t");
    }
}

static spoon *
phil_get_right_spoon(philosopher *phil){
    int phil_no = phil->phil_id;

    if (phil_no == 0)
	return &spoons[N_PHILOSOPHER - 1];
    else
	return &spoons[phil->phil_id - 1];
}

static spoon *
phil_get_left_spoon(philosopher *phil){
    return &spoons[phil->phil_id];
}

static void phil_eat(philosopher *phil){
    spoon *left_spoon, *right_spoon;

    left_spoon = phil_get_left_spoon(phil);
    right_spoon = phil_get_right_spoon(phil);

    assert(left_spoon->phil == phil);
    assert(right_spoon->phil == phil);
    assert(left_spoon->is_used == true);
    assert(right_spoon->is_used == true);

    phil->eat_count++;

    phil_print_indent(phil);
    printf("Philosopher (id=%d) eats with spoon [%d, %d] for %d times\n",
	   phil->phil_id, left_spoon->spoon_id, right_spoon->spoon_id,
	   phil->eat_count);
}

static void
phil_release_both_spoons(philosopher *phil){
    spoon *left_spoon;
    spoon *right_spoon;

    left_spoon = phil_get_left_spoon(phil);
    right_spoon = phil_get_right_spoon(phil);

    pthread_mutex_lock(&right_spoon->mutex);
    pthread_mutex_lock(&left_spoon->mutex);

    /* for debug. see phil_get_access_both_spoons also. */
    sleep(2);
    assert(left_spoon->phil == phil);
    assert(left_spoon->is_used == true);
    assert(right_spoon->phil == phil);
    assert(right_spoon->is_used == true);

    left_spoon->phil = right_spoon->phil = NULL;
    left_spoon->is_used = right_spoon->is_used = false;

    pthread_cond_signal(&left_spoon->cv);
    pthread_mutex_unlock(&left_spoon->mutex);
    pthread_cond_signal(&right_spoon->cv);
    pthread_mutex_unlock(&right_spoon->mutex);

    phil_print_indent(phil);
    printf("Philosopher (id=%d) after mutex_lock\n", phil->phil_id);
}

static bool
phil_get_access_both_spoons(philosopher *phil){
    spoon *left_spoon;
    spoon *right_spoon;

    left_spoon = phil_get_left_spoon(phil);
    right_spoon = phil_get_right_spoon(phil);

    phil_print_indent(phil);
    printf("Philosopher (id=%d) before mutex_lock\n", phil->phil_id);

    /* Get the left spoon */
    pthread_mutex_lock(&left_spoon->mutex);
    while(left_spoon->is_used){
	    pthread_cond_wait(&left_spoon->cv,
			      &left_spoon->mutex);
    }
    assert(left_spoon->is_used == false);
    left_spoon->is_used = true;
    left_spoon->phil = phil;
    pthread_mutex_unlock(&left_spoon->mutex);
    /* done */

    /* Get the right spoon if possible */
    pthread_mutex_lock(&right_spoon->mutex);
    if (right_spoon->is_used){
	/* failed */
	pthread_mutex_lock(&left_spoon->mutex);
	left_spoon->is_used = false;
	left_spoon->phil = NULL;
	pthread_mutex_unlock(&left_spoon->mutex);
	pthread_mutex_unlock(&right_spoon->mutex);
	return false;
    }else{
	/* succeeded */

	/*
	 * To make surer there is no bug, wait for two seconds
	 * (longest sleep in the whole program) and prove that
	 * other philosopher doesn't make any changes in the
	 * right_spoon conditions during the wait.
	 */
	sleep(2);
	assert(left_spoon->phil == phil);
	assert(left_spoon->is_used == true);
	assert(right_spoon->phil == NULL);
	assert(right_spoon->is_used == false);

	right_spoon->phil = phil;
	right_spoon->is_used = true;
	pthread_mutex_unlock(&right_spoon->mutex);
	return true;
    }
}

static void *
phil_fn(void *arg){
    philosopher *self = (philosopher *) arg;
    bool ready_to_eat = false;

    phil_print_indent(self);
    printf("Philosopher (id=%d) has launched\n", self->phil_id);

    while(1){
	if ((ready_to_eat = phil_get_access_both_spoons(self)) == true){
	    phil_eat(self);
	    phil_release_both_spoons(self);
	    sleep(1);
	}else{
	    phil_print_indent(self);
	    printf("Philosopher (id=%d) will sleep\n", self->phil_id);
	    sleep(1);
	}
    }
}

int
main(int argc, char **argv){
    int i = 0;
    pthread_attr_t attr;

    for (i = 0; i < N_PHILOSOPHER; i++){
	spoons[i].spoon_id = i;
	spoons[i].is_used = false;
	spoons[i].phil = NULL;
	pthread_mutex_init(&spoons[i].mutex, NULL);
	pthread_cond_init(&spoons[i].cv, NULL);
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,
				PTHREAD_CREATE_DETACHED);
    for (i = 0; i < N_PHILOSOPHER; i++){
	phils[i].phil_id = i;
	phils[i].eat_count = 0;
	pthread_create(&phils[i].thread_handle, &attr,
		       phil_fn, &phils[i]);
    }

    pthread_exit(0);

    return 0;
}
