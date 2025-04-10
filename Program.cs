var builder = WebApplication.CreateBuilder(args);

// CORS ekle
builder.Services.AddCors(options =>
{
    options.AddPolicy("AllowReactApp",
        policy => policy
            .WithOrigins("http://localhost:3000",
                "http://localhost:3001",
                "http://localhost:3009", // Þu an çalýþtýðýn port
                "http://127.0.0.1:3006") // React uygulamanýn çalýþtýðý adres
            .AllowAnyHeader()
            .AllowAnyMethod());
});

// Diðer servisler buraya...
builder.Services.AddControllers();

var app = builder.Build();

// Middleware içine CORS'u kullanmak için ekle:
app.UseCors("AllowReactApp");

app.UseHttpsRedirection();

app.UseAuthorization();

app.MapControllers();

app.Run();
