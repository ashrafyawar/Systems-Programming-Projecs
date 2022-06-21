// CHEFS
for (;;){
    sem_wait(my_semaphore);
    if (some condition){ // if initially wholeseller hasn't been put any ingredients yet (Enters only onces in the whole process's lifetime).
        // do something...
        sem_post(my_semaphore);
        continue;
    }
    else if (some condition){// if wholerseller has done bringing all the ingredients (Enters only onces in the whole process's lifetime).
        // do something...
        sem_post(my_semaphore);
        return 0;
    }else if (some condition){// if the shared memory contains my ingredients.
        // do something...
        sem_post(my_semaphore); 
    }else{ // if none of above occures. THEN increment the semaphore.
        //do something...
        sem_post(my_semaphore);
    }
}

// WHOLESELLER
for (;;){
    // do something...
    sem_wait(sp);
    if (some condition){ // if chef's haven't been emptied the shared memory yet THEN increment the semaphore.
        sem_post(sp);
        continue;
    }
    //do something...
    sem_post(sp);
    // do something...
}