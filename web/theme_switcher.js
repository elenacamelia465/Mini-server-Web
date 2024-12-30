function toggleTheme() {
    const body = document.body;
    const main = document.querySelector('main'); 
    const currentTheme = body.getAttribute('data-theme');

    if (currentTheme === 'dark') {
        
        body.setAttribute('data-theme', 'light');
        body.style.backgroundColor = '#f4f4f9';
        body.style.color = '#333';
        main.style.backgroundColor = '#fff'; 
        main.style.color = '#333'; 
    } else {
        
        body.setAttribute('data-theme', 'dark');
        body.style.backgroundColor = '#333';
        body.style.color = '#f4f4f9';
        main.style.backgroundColor = '#444'; 
        main.style.color = '#f4f4f9'; 
    }
}
