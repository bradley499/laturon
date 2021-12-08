/**
 * Boiler plate code to use for asynchronous communication between WASM and JavaScript,
 * DO NOT: change the function names as they are linked to `src/interact.c` by eternal
 * function calls, done via `EM_JS`; allowing for asynchronous inputs and outputs.
 */

var userInputState = 0;
var userInputString = null;

function getUserInput()
{
    return userInputString; // Return user input value!
}

function resetUserInputState() {
    userInputState = 0; // Set the user input state to `0`.
    userInputString = null;
}

function requestUserInput(message) {
    resetUserInputState();
    userInputString = window.prompt(message); // Request user input, and send an accompanying message to show to the user's dialogue.
    userInputState = (userInputString != null); // Validates whether user input was returned.
}

function getUserInputState() {
    return (userInputState == 1);
}

function outputMessage(message)
{
    console.log(message);
}
