
document.addEventListener('DOMContentLoaded', () => {
    console.log("download_test.js loaded successfully!");
    const body = document.body;

    
    const newElement = document.createElement('p');
    newElement.textContent = "JavaScript file download and execution succeeded!";
    newElement.style.color = "green";
    newElement.style.fontWeight = "bold";
    newElement.style.textAlign = "center";

    body.appendChild(newElement);
});
