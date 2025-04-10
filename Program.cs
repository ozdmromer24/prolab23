var builder = WebApplication.CreateBuilder(args);

// CORS ekle
builder.Services.AddCors(options =>
{
    options.AddPolicy("AllowReactApp",
        policy => policy
            .WithOrigins("http://localhost:3000",
                "http://localhost:3001",
                "http://localhost:3009", // �u an �al��t���n port
                "http://127.0.0.1:3006") // React uygulaman�n �al��t��� adres
            .AllowAnyHeader()
            .AllowAnyMethod());
});

// Di�er servisler buraya...
builder.Services.AddControllers();

var app = builder.Build();

// Middleware i�ine CORS'u kullanmak i�in ekle:
app.UseCors("AllowReactApp");

app.UseHttpsRedirection();

app.UseAuthorization();

app.MapControllers();

app.Run();
