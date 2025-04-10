using Microsoft.AspNetCore.Mvc;
using UlasimSistemi.Models;
using UlasimSistemi.Services;

[Route("api/[controller]")]
[ApiController]
public class RotaController : ControllerBase
{
	private readonly RotaServisi _rotaServisi;

	public RotaController()
	{
		var (duraklar, taksiBilgisi) = VeriOku.VeriYukle();
		_rotaServisi = new RotaServisi(duraklar, taksiBilgisi);
	}

	[HttpPost("hesapla")]
	public IActionResult RotayiHesapla([FromBody] RotaIstek istek)
	{
		var sonuc = _rotaServisi.DijkstraRotaHesapla(istek.baslangic, istek.hedef, istek.yolcuTipi);
		return Ok(sonuc);
	}
}

public class RotaIstek
{
	public Konum baslangic { get; set; }
	public Konum hedef { get; set; }
	public string yolcuTipi { get; set; }
}
