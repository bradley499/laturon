/**
 * Boiler plate code to use for asynchronous communication between WASM and
 * JavaScript. DO NOT: change the function names as they are linked to
 * `src/interact.c` by external function calls, done via `EM_JS`; allowing for
 *  asynchronous inputs and outputs.
 */

self.postMessage({
    "type": "loading",
    "state": 0
});

var userInputString = null;
var userInputState = 0;

self.addEventListener("message", function(e) {
    try {
        e.preventDefault();
        e = e.data;
        if (e["type"] == "input") {
            try {
                if (e["state"] == 1 && userInputString == null) {
                    userInputString = e["message"];
                    userInputState = 1;
                } else {
                    throw new Error("input");
                }
            } catch {
                outputMessage("An error occurred when receiving an input.", 2);
            }
        } else if (e["type"] == "sourceCode") {
            updateSourceFile(e["data"]);
        }
    } catch {
        outputMessage("An unexpected value was wrongly received.", 2);
    }
}, false);

function getUserInput() {
    return userInputString; // Return user input value!
}

function resetUserInputState() {
    userInputState = 0; // Set the user input state to `0`.
    userInputString = null;
}

function requestUserInput(message) {
    resetUserInputState();
    self.postMessage({
        "type": "input",
        "state": 0,
        "message": message
    });
}

function getUserInputState() {
    return (userInputState == 1);
}

function outputMessage(message, type) {
    self.postMessage({
        "type": "output",
        "state": type,
        "message": message
    });
}

function initialLoadComplete() {
    self.postMessage({
        "type": "loading",
        "state": 1
    });
}

function error(code) {
    const errors = ["An unknown error occurred.", "Failed to allocate memory.", "Logical operation failed.", "Failed to convert to another type.", "Failed to cleanup variables outside of current scope.", "Failed to execute operation.", "An invalid reference to a call stack scope occurred.", "Failed to correctly read in user input string.", "A reference to a compound literal does not exist.", "An array routine was not given an array to operate on.", "A reference to an item within an array that is out of range.", "Failed to perform an operation on source file."];
    try {
        outputMessage(errors[code], 2);
    } catch {
        error(0);
    }
}

function operationState(state) {
    self.postMessage({
        "type": "operating",
        "state": state
    });
}