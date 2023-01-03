
# Communication and Computing Course Assigment 4


## Authors

- [@Yoad Tamar](https://github.com/YoadTamar)
- [@Lior Vinman](https://github.com/liorvi35)

## General Description
This is assignment 4 in "Communication and Computing" Course course at Ariel University.
We were asked to implement a ping program.
- in the first part we were ask to implement a simple ping: <br>
  The user will run the program (ping.c) with the ip. The program will send an ICMP ECHO REQUEST to the host, and when receiving ICMP-ECHOREPLY, the program will send the next ICMP ECHO REQUEST (no need to stop). 
  This code can't handle timeout, and it will just wait till it gets back reply.
- in the second part we were ask to implement a "better" ping:
   The user will run the program (better_ping.c) with the ip. It will do the same thing as before, but now we will handle the timeout.
   The better ping will run a second program that hold a timer (TCP connection on port 3000) to ensure that if we don’t receive an ICMP-ECHO-REPLY after sending an ICMP-REQUEST for 10 seconds, It will exit and print <br> “server < ip > cannot be reached.”

## Run The Project 

#### To open the project:

Clone the project

```bash
  git clone https://github.com/YoadTamar/cnc_assignment4.git
```
#### To run the project:

Open terminal where you save the project files.

#### build the project:
```bash
make all
```
#### To run the first part:
```bash
sudo ./PartA <ip>
```
#### To run the second part:
```bash
sudo ./PartB <ip>
```

## Tests

The test made with JUnit 5.
We test two thing - the memory and the correctness of the classes.

We test the memory with `JvmUtilities` and `pom.xml` which implemets the `JOL` interface.

We test the classes `ConcreteMember` and `GroupAdmin` methods.
For each method we create his own test.

#### For example - memory:

![testmemory_photo](https://user-images.githubusercontent.com/119599940/210111443-7bb62884-1cd0-432f-ab3b-3b9f9a05dc52.png)


#### For example - method:

![test_photo](https://user-images.githubusercontent.com/119599940/210111176-5a90fa9a-ef4c-488b-9dfc-a1d4f8470abd.png)

## 🛠 Skills
C , 

