import pathlib
import pytest
import git


def get_git_root(path) -> pathlib.Path:
    git_repo = git.Repo(path, search_parent_directories=True)
    git_root = git_repo.git.rev_parse("--show-toplevel")
    return pathlib.Path(git_root)


@pytest.fixture(scope="function")
def data_path() -> pathlib.Path:
    """
    Provides path to test data
    """
    path = get_git_root(__file__).resolve() / "tests" / "pymetkit" / "experimental" / "data"
    assert path.exists()
    return path
