//weather.js
const apiKey = "YOUR_API_KEY"; 
const apiUrl = "https://api.openweathermap.org/data/2.5/weather";

async function getWeather(city) {
    try {
        const response = await fetch(`${apiUrl}?q=${city}&appid=${apiKey}&units=metric`);
        if (!response.ok) {
            throw new Error(`City not found: ${city}`);
        }
        const weatherData = await response.json();
        displayWeather(weatherData);
    } catch (error) {
        console.error("Error fetching weather data:", error);
        alert(error.message);
    }
}

function displayWeather(data) {
    const { name, main, weather } = data;
    console.log(`Weather in ${name}:`);
    console.log(`Temperature: ${main.temp}°C`);
    console.log(`Description: ${weather[0].description}`);
    console.log(`Humidity: ${main.humidity}%`);
}


document.addEventListener("DOMContentLoaded", () => {
    const cityInput = document.getElementById("city-input");
    const searchButton = document.getElementById("search-button");

    searchButton.addEventListener("click", () => {
        const city = cityInput.value.trim();
        if (city) {
            getWeather(city);
        } else {
            alert("Please enter a city name.");
        }
    });
});
