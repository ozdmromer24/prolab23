using IzmitUlasimAPI.Models;
using IzmitUlasimAPI.Services;
using Microsoft.AspNetCore.Mvc;

namespace IzmitUlasimAPI.Controllers;

[Route("api/[controller]")]
[ApiController]
public class DurakController : ControllerBase
{
    private readonly DurakService _service;

    public DurakController()
    {
        _service = new DurakService();
    }

    [HttpGet]
    public ActionResult<List<Durak>> GetAll() => _service.GetAll();

    [HttpGet("{id}")]
    public ActionResult<Durak> GetById(string id)
    {
        var durak = _service.GetById(id);
        if (durak == null) return NotFound();
        return durak;
    }

    [HttpPost("enyakin")]
    public ActionResult<Durak> EnYakinDurak([FromBody] Konum konum)
    {
        var durak = _service.EnYakinDuragiBul(konum);
        return durak;
    }
}
