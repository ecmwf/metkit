from pathlib import Path
import pytest


@pytest.fixture(scope="function")
def data_path() -> Path:
    """
    Provides path to test data
    """
    path = Path(__file__).parent / "data"
    assert path.exists()
    return path
