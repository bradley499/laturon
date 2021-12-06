/**
 * Boiler plate code to use for asynchronous communication between WASM and JavaScript,
 * DO NOT: change the function names as they are linked to `src/javascript.c` by eternal
 * function calls, done via `EM_JS`; allowing for asynchronous inputs and outputs.
 */

var userInputState = 0;

function getUserInput()
{
    return "success!"; // Example return value - change to return user input value!
}

function resetUserInputState() {
    userInputState = 0; // Set the user input state to `0`.
}

function requestUserInput(message) {
    resetUserInputState();
    console.info("message: " + message); // Request user input, and send an accompanying message to show to the user's dialogue. 
    setTimeout(() => {
        userInputState = 1; // Example timer to test out asynchronous inputs and outputs. Please remove!
    }, 2000);
}

function getUserInputState() {
    return (userInputState == 1);
}

function outputMessage(message)
{
    console.log(message);
}
