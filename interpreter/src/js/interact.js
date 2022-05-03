/**
 * Boiler plate code to use for asynchronous communication between WASM and
 * JavaScript. DO NOT: change the function names as they are linked to
 * `src/interact.c` by external function calls, done via `EM_JS`; allowing for
 *  asynchronous inputs and outputs.
 */

Module.noInitialRun = true;

var isInitial = null;
var userInputString = null;
var userInputState = 0;
function showVersion(){
    return new Promise((resolve, reject) => {
        setInterval(function () {
            if (isInitial != null) {
                if (isInitial){
                    resolve();
                } else {
                    reject();
                }
            }
        }, 100);
    });
}

self.addEventListener("message", async function(e) {
    try {
        e.preventDefault();
        e = e.data;
        if (e["type"] == "input") {
            try {
                if (e["state"] == 1 && userInputString == null) {
                    userInputString = e["message"] || "";
                    userInputState = 1;
                } else {
                    throw new Error("input");
                }
            } catch {
                outputMessage("An error occurred when receiving an input.", 2);
            }
        } else if (e["type"] == "sourceCode") {
            await showVersion().then(function(){}).catch(function(){}).finally(function() {
                updateSourceFile(e["data"]);
            });
        } else if (e["type"] == "startup") {
            ready(e["data"]);
        }
    } catch (err) {
        if (err["name"] == "ExitStatus") return;
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

function requestUserInput() {
    resetUserInputState();
    self.postMessage({
        "type": "input",
        "state": 0,
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

function setLoadState(state) {
    self.postMessage({
        "type": "loading",
        "state": state
    });
}

function error(code, line) {
    const errors = ["An unknown error occurred.", "Failed to allocate memory.", "Logical operation failed.", "Failed to convert to another type.", "Attempted to use a value from a variable has not been assigned.", "Failed to cleanup variables outside of current scope.", "Failed to execute operation.", "An invalid reference to a call stack scope occurred.", "Failed to correctly read in user input string.", "A reference to a compound literal does not exist.", "An array routine was not given an array to operate on.", "A reference to an item within an array that is out of range.", "Failed to perform an operation on source file.", "The source provided has invalid syntax.", "The total amount of stack memory available to execute your program has been reached.", "Too many functions or variables are declared within your program to be handled within memory.", "Zero division error.", "You are attempting to use syntax that has not been finalised yet."];
    try {
        let error = errors[code];
        if (error == undefined){
            throw new Error("Error");
        }
        if (line > 0) {
            error = error + " On line: " + line;
        }
        outputMessage(error, 2);
    } catch {
        error(0, line);
    }
}

function operationState(state) {
    self.postMessage({
        "type": "operating",
        "state": state
    });
}

function ready(state) {
    isInitial = state[0];
    if (isInitial || state[1]) {
        self.postMessage({
            "type": "loading",
            "state": 0
        });
    } else {
        self.postMessage({
            "type": "loading",
            "state": 1
        });
    }
}

function terminate() {
    self.postMessage({
        "type": "terminate",
        "state": 1
    });
}


Module.onRuntimeInitialized = async function(){
    await showVersion().then(function(){
        Module.ccall("versioning", "number", [], []);
    }).catch(function(){});
    setLoadState(1);
}