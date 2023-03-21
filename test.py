import sys
import getopt
import subprocess
import signal
import difflib


HOST_OPT = '-h'
PORT_OPT = '-p'
MODE_OPT = '-m'
HELP_OPT = '--help'

HOST=None
PORT=None
MODE=None

PROCESS=None


class Color:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'



def sigint_handler(signum, frame):
    if PROCESS is not None: 
        PROCESS.kill()


# Register the SIGINT handler function
signal.signal(signal.SIGINT, sigint_handler)


def usage():
    print(f"Usage: python test.py -h <host> -p <port> -m <mode>")



opts, args = getopt.getopt(sys.argv[1:], "h:p:m:", ['help'])

for opt, arg in opts:
    if opt == HOST_OPT:
        HOST = arg
    elif opt == PORT_OPT:
        PORT = arg
    elif opt == MODE_OPT:
        MODE = arg
    elif opt == HELP_OPT:
        print("Usage: python test.py -h <host> -p <port> -m <mode>")
        sys.exit(0)
    else:
        print(
            "Too few or too many arguments. Please use --help to get more information.")
        sys.exit(0)

def test(host=HOST, port=PORT, mode=MODE):
    def configWrapper(testingFunc):
        def wrapper():
            output = None

            PROCESS = subprocess.Popen(["./ipkcpc", "-h", host, "-p",
                                        port, "-m", mode], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            input, expect = testingFunc()

            if MODE == 'tcp':
                expect = "You are connected to the server.\n" + \
                    expect + "You are disconnected from the server.\n"

            try:

                # Write the input to stdin
                PROCESS.stdin.write(input.encode('utf-8'))

                # Read the output from stdout
                output,err = PROCESS.communicate()
                output = output.decode('utf-8') 


                PROCESS.terminate()
            except subprocess.TimeoutExpired:
                print(err, output)
                PROCESS.terminate()

            # Print the return code
            # Create a SequenceMatcher object
            matcher = difflib.SequenceMatcher(
                None, output, expect)

            # Generate a list of differences
            diffs = matcher.get_opcodes()

            # Output the differences
            for opcode, i1, i2, j1, j2 in diffs:
                if opcode == 'insert':
                    print(f"{Color.FAIL}Missing response: {expect[j1:j2]}{Color.ENDC}")
                elif opcode == 'delete':
                    print(
                        f"{Color.FAIL}Unknown response: {output[i1:i2]}{Color.ENDC}")
                elif opcode == 'replace':
                    print(f"{Color.FAIL}Invalid response. Replace",
                          output[i1:i2], "with", expect[j1:j2], Color.ENDC)

            if output == expect:
                print(f"{testingFunc.__name__} {Color.OKGREEN}OK{Color.ENDC}")
            else:
                print(f"Expected output:\n{expect}\nYour output:\n{output}")
            

        return wrapper
    return configWrapper


@test()
def easy_test_tcp():
    EXPECT = "HELLO\nRESULT 7\nBYE\n"
    INPUT = "HELLO\nSOLVE (+ 2 5)\nBYE\n"

    return INPUT, EXPECT


@test()
def multiple_test_tcp():
    EXPECT = "HELLO\nRESULT 6\nRESULT 6/23\nRESULT -136\nBYE\n"
    INPUT = "HELLO\nSOLVE (+ 2 (* 2 2))\nSOLVE (/ 12 (* 23 2))\nSOLVE (- 26 (* 54 3))\nBYE\n"

    return INPUT, EXPECT

@test()
def hard_test_tcp():
    EXPECT = "HELLO\nRESULT 55/2\nBYE\n"
    INPUT = "HELLO\nSOLVE (+ 2 (* 34 (/ 3 4)))\nBYE\n"

    return INPUT, EXPECT

@test()
def easy_test_udp():
    EXPECT = "OK:4\n"
    INPUT = "(+ 2 2)\n"

    return INPUT, EXPECT


@test()
def multiple_test_udp():
    EXPECT = "OK:6\nOK:6/23\nOK:-136\n"
    INPUT = "(+ 2 (* 2 2))\n(/ 12 (* 23 2))\n(- 26 (* 54 3))\n"

    return INPUT, EXPECT


@test()
def hard_test_udp():
    EXPECT = "OK:55/2\n"
    INPUT = "(+ 2 (* 34 (/ 3 4)))\n"

    return INPUT, EXPECT



if MODE == 'tcp':
    easy_test_tcp()
    multiple_test_tcp()
    hard_test_tcp()
else:
    easy_test_udp()
    multiple_test_udp()
    hard_test_udp()



    
